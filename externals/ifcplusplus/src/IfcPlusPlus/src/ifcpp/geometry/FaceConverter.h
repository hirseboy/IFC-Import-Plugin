/* -*-c++-*- IfcQuery www.ifcquery.com
*
MIT License

Copyright (c) 2017 Fabian Gerold

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <ifcpp/geometry/GeometrySettings.h>
#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/StatusCallback.h>
#include <ifcpp/model/UnitConverter.h>

#include <IfcAdvancedFace.h>
#include <IfcCurveBoundedPlane.h>
#include <IfcCurveBoundedSurface.h>
#include <IfcCylindricalSurface.h>
#include <IfcFace.h>
#include <IfcFaceBound.h>
#include <IfcPlane.h>
#include <IfcRationalBSplineSurfaceWithKnots.h>
#include <IfcRectangularTrimmedSurface.h>
#include <IfcSurfaceOfLinearExtrusion.h>
#include <IfcSurfaceOfRevolution.h>
#include <IfcSweptSurface.h>

#include "IncludeCarveHeaders.h"
#include "GeometryInputData.h"
#include "CurveConverter.h"
#include "SplineConverter.h"
#include "ProfileCache.h"
#include "Sweeper.h"

class SurfaceProxy
{
public:
	virtual void computePointOnSurface(const vec3& point_in, vec3& point_out) = 0;
};

class SurfaceProxyLinear : public SurfaceProxy
{
public:
	virtual void computePointOnSurface(const vec3& point_in, vec3& point_out)
	{
		point_out = m_surface_matrix*point_in;
	}
	carve::math::Matrix m_surface_matrix;
};

class FaceConverter : public StatusCallback
{
public:
	enum ShellType { SHELL_TYPE_UNKONWN, OPEN_SHELL, CLOSED_SHELL };

	shared_ptr<GeometrySettings>	m_geom_settings;
	shared_ptr<UnitConverter>		m_unit_converter;
	shared_ptr<CurveConverter>		m_curve_converter;
	shared_ptr<SplineConverter>		m_spline_converter;
	shared_ptr<Sweeper>				m_sweeper;
	shared_ptr<ProfileCache>		m_profile_cache;

	FaceConverter( shared_ptr<GeometrySettings>& gs, shared_ptr<UnitConverter>& uc, shared_ptr<CurveConverter>& cc, shared_ptr<SplineConverter>& sc, shared_ptr<Sweeper>& sw, shared_ptr<ProfileCache>&	profile_cache )
		: m_geom_settings( gs ), m_unit_converter( uc ), m_curve_converter( cc ), m_spline_converter( sc ), m_sweeper( sw ), m_profile_cache(profile_cache)
	{
	}

	virtual ~FaceConverter(){}

	void convertIfcSurface( const shared_ptr<IfcSurface>& surface, shared_ptr<ItemShapeData>& item_data, shared_ptr<SurfaceProxy>& surface_proxy, double plane_span_default = -1)
	{
		//ENTITY IfcSurface ABSTRACT SUPERTYPE OF(ONEOF(IfcBoundedSurface, IfcElementarySurface, IfcSweptSurface))

		shared_ptr<IfcBoundedSurface> bounded_surface = dynamic_pointer_cast<IfcBoundedSurface>( surface );
		if( bounded_surface )
		{
			// ENTITY IfcBoundedSurface ABSTRACT SUPERTYPE OF(ONEOF(IfcBSplineSurface, IfcCurveBoundedPlane, IfcCurveBoundedSurface, IfcRectangularTrimmedSurface))
			if( dynamic_pointer_cast<IfcBSplineSurface>( bounded_surface ) )
			{
				if( dynamic_pointer_cast<IfcRationalBSplineSurfaceWithKnots>( bounded_surface ) )
				{
					shared_ptr<IfcRationalBSplineSurfaceWithKnots> nurbs_surface = dynamic_pointer_cast<IfcRationalBSplineSurfaceWithKnots>( bounded_surface );
					if( nurbs_surface )
					{
						shared_ptr<carve::input::PolylineSetData> polyline_data( new carve::input::PolylineSetData() );
						m_spline_converter->convertIfcBSplineSurface( nurbs_surface, polyline_data );
						if( polyline_data->getVertexCount() > 1 )
						{
							item_data->m_polylines.push_back( polyline_data );
						}
					}
				}
			}
			else if( dynamic_pointer_cast<IfcCurveBoundedPlane>( bounded_surface ) )
			{
				// ENTITY IfcCurveBoundedPlane SUBTYPE OF IfcBoundedSurface;
				shared_ptr<IfcCurveBoundedPlane> curve_bounded_plane = dynamic_pointer_cast<IfcCurveBoundedPlane>( bounded_surface );
				shared_ptr<TransformData> curve_bounded_plane_matrix;
				shared_ptr<IfcPlane>& basis_surface = curve_bounded_plane->m_BasisSurface;
				if( basis_surface )
				{
					shared_ptr<IfcAxis2Placement3D>& basis_surface_placement = basis_surface->m_Position;

					if( basis_surface_placement )
					{
						m_curve_converter->getPlacementConverter()->convertIfcAxis2Placement3D( basis_surface_placement, curve_bounded_plane_matrix );
					}
				}

				// convert outer boundary
				shared_ptr<IfcCurve>& outer_boundary = curve_bounded_plane->m_OuterBoundary;
				std::vector<std::vector<vec3> > face_loops;
				face_loops.push_back( std::vector<vec3>() );
				std::vector<vec3>& outer_boundary_loop = face_loops.back();
				std::vector<vec3> segment_start_points;
				m_curve_converter->convertIfcCurve( outer_boundary, outer_boundary_loop, segment_start_points, true );

				// convert inner boundaries
				std::vector<shared_ptr<IfcCurve> >& vec_inner_boundaries = curve_bounded_plane->m_InnerBoundaries;			//optional
				for( auto& inner_boundary : vec_inner_boundaries )
				{
					if( !inner_boundary )
					{
						continue;
					}
					face_loops.push_back( std::vector<vec3>() );
					std::vector<vec3>& inner_boundary_loop = face_loops.back();
					std::vector<vec3> segment_start_points;
					m_curve_converter->convertIfcCurve( inner_boundary, inner_boundary_loop, segment_start_points, true );
				}

				double CARVE_EPSILON = m_geom_settings->getEpsilonCoplanarDistance();
				PolyInputCache3D poly_cache(CARVE_EPSILON);
				bool mergeAlignedEdges = true;
				GeomProcessingParams params( m_geom_settings, outer_boundary.get(),  this );
				MeshUtils::createTriangulated3DFace( face_loops, poly_cache, params );
				item_data->addOpenPolyhedron( poly_cache.m_poly_data, CARVE_EPSILON );
				item_data->applyTransformToItem( curve_bounded_plane_matrix );
			}
			else if( dynamic_pointer_cast<IfcCurveBoundedSurface>( bounded_surface ) )
			{
				shared_ptr<IfcCurveBoundedSurface> curve_bounded_surface = dynamic_pointer_cast<IfcCurveBoundedSurface>( bounded_surface );
				shared_ptr<IfcSurface>& basis_surface = curve_bounded_surface->m_BasisSurface;
				if( basis_surface )
				{
					convertIfcSurface( basis_surface, item_data, surface_proxy );
				}


				//std::vector<shared_ptr<IfcBoundaryCurve> >& vec_boundaries = curve_bounded_surface->m_Boundaries;
				//bool implicit_outer = curve_bounded_surface->m_ImplicitOuter;

				// TODO: implement
#ifdef _DEBUG
				std::cout << "IfcCurveBoundedSurface boundaries not implemented." << std::endl;
#endif
			}
			else if( dynamic_pointer_cast<IfcRectangularTrimmedSurface>( bounded_surface ) )
			{
				shared_ptr<IfcRectangularTrimmedSurface> rectengular_trimmed_surface = dynamic_pointer_cast<IfcRectangularTrimmedSurface>( bounded_surface );

				shared_ptr<IfcSurface>& basis_surface = rectengular_trimmed_surface->m_BasisSurface;
				if( basis_surface )
				{
					convertIfcSurface( basis_surface, item_data, surface_proxy );
				}

				//shared_ptr<IfcParameterValue>& u1 = rectengular_trimmed_surface->m_U1;
				//shared_ptr<IfcParameterValue>& v1 = rectengular_trimmed_surface->m_V1;
				//shared_ptr<IfcParameterValue>& u2 = rectengular_trimmed_surface->m_U2;
				//shared_ptr<IfcParameterValue>& v2 = rectengular_trimmed_surface->m_V2;
				//bool u_sense = rectengular_trimmed_surface->m_Usense;
				//bool v_sense = rectengular_trimmed_surface->m_Vsense;
				// TODO: implement
#ifdef _DEBUG
				std::cout << "IfcRectangularTrimmedSurface U1, V1, U2, V2 not implemented." << std::endl;
#endif
			}
			return;
		}

		shared_ptr<IfcElementarySurface> elementary_surface = dynamic_pointer_cast<IfcElementarySurface>( surface );
		if( elementary_surface )
		{
			//ENTITY IfcElementarySurface	ABSTRACT SUPERTYPE OF(ONEOF(IfcCylindricalSurface, IfcPlane))
			shared_ptr<IfcAxis2Placement3D>& elementary_surface_placement = elementary_surface->m_Position;

			shared_ptr<TransformData> elementary_surface_transform;
			if( elementary_surface_placement )
			{
				m_curve_converter->getPlacementConverter()->convertIfcAxis2Placement3D( elementary_surface_placement, elementary_surface_transform );
			}

			shared_ptr<SurfaceProxyLinear> proxy_linear( new SurfaceProxyLinear() );
			if( elementary_surface_transform )
			{
				proxy_linear->m_surface_matrix = elementary_surface_transform->m_matrix;
			}
			surface_proxy = proxy_linear;

			shared_ptr<IfcPlane> elementary_surface_plane = dynamic_pointer_cast<IfcPlane>( elementary_surface );
			if( elementary_surface_plane )
			{
				//  1----0     create big rectangular plane
				//  |    |     ^ y
				//  |    |     |
				//  2----3     ---> x
				{
					double plane_span = HALF_SPACE_BOX_SIZE*m_unit_converter->getCustomLengthFactor();
					if( plane_span_default > 0 )
					{
						plane_span = plane_span_default;
					}
					shared_ptr<carve::input::PolylineSetData> polyline_data( new carve::input::PolylineSetData() );
					polyline_data->beginPolyline();
					polyline_data->addVertex( proxy_linear->m_surface_matrix*carve::geom::VECTOR( plane_span, plane_span, 0.0 ) );
					polyline_data->addVertex( proxy_linear->m_surface_matrix*carve::geom::VECTOR( -plane_span, plane_span, 0.0 ) );
					polyline_data->addVertex( proxy_linear->m_surface_matrix*carve::geom::VECTOR( -plane_span, -plane_span, 0.0 ) );
					polyline_data->addVertex( proxy_linear->m_surface_matrix*carve::geom::VECTOR( plane_span, -plane_span, 0.0 ) );

					polyline_data->addPolylineIndex( 0 );
					polyline_data->addPolylineIndex( 1 );
					polyline_data->addPolylineIndex( 2 );
					polyline_data->addPolylineIndex( 3 );
					item_data->m_polylines.push_back( polyline_data );
				}
				return;
			}

			shared_ptr<IfcCylindricalSurface> cylindrical_surface = dynamic_pointer_cast<IfcCylindricalSurface>( elementary_surface );
			if( cylindrical_surface )
			{
				shared_ptr<IfcPositiveLengthMeasure> cylindrical_surface_radius = cylindrical_surface->m_Radius;
				double circle_radius = cylindrical_surface_radius->m_value;

				int num_segments = m_geom_settings->getNumVerticesPerCircleWithRadius(circle_radius);
				double start_angle = 0.0;
				double opening_angle = M_PI*2.0;
				const double circle_center_x = 0.0;
				const double circle_center_y = 0.0;

				std::vector<vec2> circle_points;
				GeomUtils::addArcWithEndPoint( circle_points, circle_radius, start_angle, opening_angle, circle_center_x, circle_center_y, num_segments );

				// apply position and insert points
				shared_ptr<carve::input::PolylineSetData> polyline_data( new carve::input::PolylineSetData() );
				polyline_data->beginPolyline();
				for( size_t i = 0; i < circle_points.size(); ++i )
				{
					vec2& point = circle_points[i];
					vec3 point3d( carve::geom::VECTOR( point.x, point.y, 0 ) );
					if( elementary_surface_transform )
					{
						polyline_data->addVertex( elementary_surface_transform->m_matrix*point3d );
					}
					polyline_data->addPolylineIndex( i );
				}
				item_data->m_polylines.push_back( polyline_data );
				return;
			}

			throw UnhandledRepresentationException( surface );
		}

		shared_ptr<IfcSweptSurface> swept_surface = dynamic_pointer_cast<IfcSweptSurface>( surface );
		if( dynamic_pointer_cast<IfcSweptSurface>( surface ) )
		{
			// ENTITY IfcSweptSurface	ABSTRACT SUPERTYPE OF(ONEOF(IfcSurfaceOfLinearExtrusion, IfcSurfaceOfRevolution))
			shared_ptr<IfcProfileDef>& swept_surface_profile = swept_surface->m_SweptCurve;
			shared_ptr<ProfileConverter> profileCon = m_profile_cache->getProfileConverter(swept_surface_profile);
			const std::vector<std::vector<vec2> >& swept_profile = profileCon->getCoordinates();

			shared_ptr<IfcAxis2Placement3D>& swept_surface_placement = swept_surface->m_Position;

			shared_ptr<TransformData> swept_surface_transform;
			if( swept_surface_placement )
			{
				m_curve_converter->getPlacementConverter()->convertIfcAxis2Placement3D( swept_surface_placement,  swept_surface_transform );
			}

			shared_ptr<IfcSurfaceOfLinearExtrusion> linear_extrusion = dynamic_pointer_cast<IfcSurfaceOfLinearExtrusion>( swept_surface );
			if( linear_extrusion )
			{
				shared_ptr<IfcDirection>& ifc_extrusion_direction = linear_extrusion->m_ExtrudedDirection;
				if( ifc_extrusion_direction )
				{
					vec3 extrusion_direction = carve::geom::VECTOR(ifc_extrusion_direction->m_DirectionRatios[0]->m_value, ifc_extrusion_direction->m_DirectionRatios[1]->m_value, ifc_extrusion_direction->m_DirectionRatios[2]->m_value);

					double factor = 1.0;
					if( linear_extrusion->m_Depth )
					{
						factor = linear_extrusion->m_Depth->m_value;
						extrusion_direction *= factor;
					}

					GeomProcessingParams params(m_geom_settings, surface.get(), this);
					m_sweeper->extrude(swept_profile, extrusion_direction, item_data, params);
				}
				
				return;
			}

			shared_ptr<IfcSurfaceOfRevolution> suface_of_revolution = dynamic_pointer_cast<IfcSurfaceOfRevolution>( swept_surface );
			if( suface_of_revolution )
			{
				// TODO: implement
#ifdef _DEBUG
				std::cout << "IfcSurfaceOfRevolution not implemented." << std::endl;
#endif
				return;
			}

			throw UnhandledRepresentationException( surface );
		}
		throw UnhandledRepresentationException( surface );
	}

	void convertIfcFaceList( const std::vector<shared_ptr<IfcFace> >& vec_faces, shared_ptr<ItemShapeData> item_data, ShellType st )
	{
		if( vec_faces.size() == 0 )
		{
			return;
		}
		double CARVE_EPSILON = m_geom_settings->getEpsilonCoplanarDistance();
		PolyInputCache3D poly_cache(CARVE_EPSILON);
		GeomProcessingParams params( m_geom_settings, nullptr,  this );
		
		for( size_t ii = 0; ii < vec_faces.size(); ++ii )
		{
			const shared_ptr<IfcFace>& ifc_face = vec_faces[ii];
			if( !ifc_face )
			{
				continue;
			}

#ifdef _DEBUG
			shared_ptr<IfcAdvancedFace> advancedFace = dynamic_pointer_cast<IfcAdvancedFace>( ifc_face );
			if( advancedFace )
			{
				int tag = advancedFace->m_tag;
				if( tag == 1261069 )
				{
					std::cout << "IfcAdvancedFace, tag=" << tag << std::endl;
				}
			}
#endif

			const std::vector<shared_ptr<IfcFaceBound> >& vec_bounds = ifc_face->m_Bounds;
			std::vector<std::vector<vec3> > face_loops;
			params.ifc_entity = ifc_face.get();
			bool mergeAlignedEdges = true;
			size_t maxNumIntersectionPoints = 200;

			for( auto it_bounds = vec_bounds.begin(); it_bounds != vec_bounds.end(); ++it_bounds )
			{
				const shared_ptr<IfcFaceBound>& face_bound = ( *it_bounds );

				if( !face_bound )
				{
					continue;
				}

				// ENTITY IfcLoop SUPERTYPE OF(ONEOF(IfcEdgeLoop, IfcPolyLoop, IfcVertexLoop))
				const shared_ptr<IfcLoop>& loop = face_bound->m_Bound;
				if( !loop )
				{
					if( it_bounds == vec_bounds.begin() )
					{
						break;
					}
					else
					{
						continue;
					}
				}

				face_loops.push_back( std::vector<vec3>() );
				std::vector<vec3>& loop_points = face_loops.back();
				m_curve_converter->convertIfcLoop( loop, loop_points );

				if( loop_points.size() < 3 )
				{
					if( it_bounds == vec_bounds.begin() )
					{
						break;
					}
					else
					{
						continue;
					}
				}

				bool orientation = true;
				if( face_bound->m_Orientation )
				{
					orientation = face_bound->m_Orientation->m_value;
				}
				if( !orientation )
				{
					std::reverse( loop_points.begin(), loop_points.end() );
				}
			}

			for( size_t iiLoop = 0; iiLoop < face_loops.size(); ++iiLoop )
			{
				std::vector<vec3>& loop = face_loops[iiLoop];
				GeomUtils::unClosePolygon(loop);
			}
			
			MeshUtils::createTriangulated3DFace( face_loops, poly_cache, params );

#ifdef _DEBUG
			if( ifc_face->m_tag == 1261069 )
			{
				//params.debugDump = true;
			}

			//if( ifc_face->m_tag == 2529 )
			//{
			//	params.debugDump = true;
			//}

			if( params.debugDump )
			{
				glm::vec4 color(0.5, 0.6, 0.7, 1.0);
				for( size_t iiLoop = 0; iiLoop < face_loops.size(); ++iiLoop )
				{
					std::vector<vec3>& loop = face_loops[iiLoop];
					GeomDebugDump::dumpPolyline(loop, color, false);
				}

				//if( ii == 34 )
				{
					PolyInputCache3D poly_cache_dump(CARVE_EPSILON);
					MeshUtils::createTriangulated3DFace(face_loops, poly_cache_dump, params);
					std::map<std::string, std::string> mesh_input_options;
					shared_ptr<carve::mesh::MeshSet<3> > meshset(poly_cache_dump.m_poly_data->createMesh(mesh_input_options, CARVE_EPSILON));
					GeomDebugDump::dumpMeshset(meshset, color, false);
				}
				GeomDebugDump::moveOffset(0.0001);
			}
#endif
		}

		// IfcFaceList can be a closed or open shell
		if( st == SHELL_TYPE_UNKONWN )
		{
			item_data->addOpenOrClosedPolyhedron( poly_cache.m_poly_data, CARVE_EPSILON );
		}
		else if( st == OPEN_SHELL )
		{
			item_data->addOpenPolyhedron( poly_cache.m_poly_data, CARVE_EPSILON );
		}
		else if( st == CLOSED_SHELL )
		{
			item_data->addClosedPolyhedron(poly_cache.m_poly_data, params);
		}
	}
};
