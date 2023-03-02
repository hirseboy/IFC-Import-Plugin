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

#include <ifcpp/IFC4X3/include/IfcBoolean.h>
#include <ifcpp/IFC4X3/include/IfcCurveBoundedPlane.h>
#include <ifcpp/IFC4X3/include/IfcCurveBoundedSurface.h>
#include <ifcpp/IFC4X3/include/IfcCylindricalSurface.h>
#include <ifcpp/IFC4X3/include/IfcFaceBound.h>
#include <ifcpp/IFC4X3/include/IfcPlane.h>
#include <ifcpp/IFC4X3/include/IfcRectangularTrimmedSurface.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceOfLinearExtrusion.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceOfRevolution.h>
#include <ifcpp/IFC4X3/include/IfcSweptSurface.h>
#include <ifcpp/IFC4X3/include/IfcPositiveLengthMeasure.h>
#include <ifcpp/IFC4X3/include/IfcRationalBSplineSurfaceWithKnots.h>

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
#include "IFCC_FaceConverter.h"

namespace IFCC {


	FaceConverter::FaceConverter( shared_ptr<GeometrySettings>& gs, shared_ptr<UnitConverter>& uc, shared_ptr<CurveConverter>& cc, shared_ptr<SplineConverter>& sc, shared_ptr<Sweeper>& sw )
		: m_geom_settings( gs ), m_unit_converter( uc ), m_curve_converter( cc ), m_spline_converter( sc ), m_sweeper( sw )
	{
	}

	void FaceConverter::convertIfcSurface( const shared_ptr<IFC4X3::IfcSurface>& surface, shared_ptr<ItemShapeData>& item_data, shared_ptr<SurfaceProxy>& surface_proxy )
	{
		//ENTITY IfcSurface ABSTRACT SUPERTYPE OF(ONEOF(IfcBoundedSurface, IfcElementarySurface, IfcSweptSurface))

		//double length_factor = m_unit_converter->getLengthInMeterFactor();
		shared_ptr<IFC4X3::IfcBoundedSurface> bounded_surface = dynamic_pointer_cast<IFC4X3::IfcBoundedSurface>( surface );
		if( bounded_surface )
		{
			// ENTITY IfcBoundedSurface ABSTRACT SUPERTYPE OF(ONEOF(IfcBSplineSurface, IfcCurveBoundedPlane, IfcCurveBoundedSurface, IfcRectangularTrimmedSurface))
			if( dynamic_pointer_cast<IFC4X3::IfcBSplineSurface>( bounded_surface ) )
			{
				if( dynamic_pointer_cast<IFC4X3::IfcRationalBSplineSurfaceWithKnots>( bounded_surface ) )
				{
					shared_ptr<IFC4X3::IfcRationalBSplineSurfaceWithKnots> nurbs_surface = dynamic_pointer_cast<IFC4X3::IfcRationalBSplineSurfaceWithKnots>( bounded_surface );
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
			else if( dynamic_pointer_cast<IFC4X3::IfcCurveBoundedPlane>( bounded_surface ) )
			{
				// ENTITY IfcCurveBoundedPlane SUBTYPE OF IfcBoundedSurface;
				shared_ptr<IFC4X3::IfcCurveBoundedPlane> curve_bounded_plane = dynamic_pointer_cast<IFC4X3::IfcCurveBoundedPlane>( bounded_surface );
				shared_ptr<TransformData> curve_bounded_plane_matrix;
				shared_ptr<IFC4X3::IfcPlane>& basis_surface = curve_bounded_plane->m_BasisSurface;
				if( basis_surface )
				{
					shared_ptr<IFC4X3::IfcAxis2Placement3D>& basis_surface_placement = basis_surface->m_Position;

					if( basis_surface_placement )
					{
						m_curve_converter->getPlacementConverter()->convertIfcAxis2Placement3D( basis_surface_placement, curve_bounded_plane_matrix );
					}
				}

				// convert outer boundary
				shared_ptr<IFC4X3::IfcCurve>& outer_boundary = curve_bounded_plane->m_OuterBoundary;
				std::vector<std::vector<vec3> > face_loops;
				face_loops.push_back( std::vector<vec3>() );
				std::vector<vec3>& outer_boundary_loop = face_loops.back();
				std::vector<vec3> segment_start_points;
				m_curve_converter->convertIfcCurve( outer_boundary, outer_boundary_loop, segment_start_points, true );

				// convert inner boundaries
				std::vector<shared_ptr<IFC4X3::IfcCurve> >& vec_inner_boundaries = curve_bounded_plane->m_InnerBoundaries;			//optional
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
//				bool mergeAlignedEdges = true;
				GeomProcessingParams params( m_geom_settings, outer_boundary.get(),  this );
				MeshUtils::createTriangulated3DFace( face_loops, poly_cache, params );
				item_data->addOpenPolyhedron( poly_cache.m_poly_data, CARVE_EPSILON );
				item_data->applyTransformToItem( curve_bounded_plane_matrix );
			}
			else if( dynamic_pointer_cast<IFC4X3::IfcCurveBoundedSurface>( bounded_surface ) )
			{
				shared_ptr<IFC4X3::IfcCurveBoundedSurface> curve_bounded_surface = dynamic_pointer_cast<IFC4X3::IfcCurveBoundedSurface>( bounded_surface );
				shared_ptr<IFC4X3::IfcSurface>& basis_surface = curve_bounded_surface->m_BasisSurface;
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
			else if( dynamic_pointer_cast<IFC4X3::IfcRectangularTrimmedSurface>( bounded_surface ) )
			{
				shared_ptr<IFC4X3::IfcRectangularTrimmedSurface> rectengular_trimmed_surface = dynamic_pointer_cast<IFC4X3::IfcRectangularTrimmedSurface>( bounded_surface );

				shared_ptr<IFC4X3::IfcSurface>& basis_surface = rectengular_trimmed_surface->m_BasisSurface;
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

		shared_ptr<IFC4X3::IfcElementarySurface> elementary_surface = dynamic_pointer_cast<IFC4X3::IfcElementarySurface>( surface );
		if( elementary_surface )
		{
			//ENTITY IfcElementarySurface	ABSTRACT SUPERTYPE OF(ONEOF(IfcCylindricalSurface, IfcPlane))
			shared_ptr<IFC4X3::IfcAxis2Placement3D>& elementary_surface_placement = elementary_surface->m_Position;

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

			shared_ptr<IFC4X3::IfcPlane> elementary_surface_plane = dynamic_pointer_cast<IFC4X3::IfcPlane>( elementary_surface );
			if( elementary_surface_plane )
			{
				//  1----0     create big rectangular plane
				//  |    |     ^ y
				//  |    |     |
				//  2----3     ---> x
				{
					double plane_span = HALF_SPACE_BOX_SIZE*m_unit_converter->getCustomLengthFactor();
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

			shared_ptr<IFC4X3::IfcCylindricalSurface> cylindrical_surface = dynamic_pointer_cast<IFC4X3::IfcCylindricalSurface>( elementary_surface );
			if( cylindrical_surface )
			{
				shared_ptr<IFC4X3::IfcPositiveLengthMeasure> cylindrical_surface_radius = cylindrical_surface->m_Radius;
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

		shared_ptr<IFC4X3::IfcSweptSurface> swept_surface = dynamic_pointer_cast<IFC4X3::IfcSweptSurface>( surface );
		if( dynamic_pointer_cast<IFC4X3::IfcSweptSurface>( surface ) )
		{
			// ENTITY IfcSweptSurface	ABSTRACT SUPERTYPE OF(ONEOF(IfcSurfaceOfLinearExtrusion, IfcSurfaceOfRevolution))
			//shared_ptr<IfcProfileDef>& swept_surface_profile = swept_surface->m_SweptCurve;
			shared_ptr<IFC4X3::IfcAxis2Placement3D>& swept_surface_placement = swept_surface->m_Position;

			shared_ptr<TransformData> swept_surface_transform;
			if( swept_surface_placement )
			{
				m_curve_converter->getPlacementConverter()->convertIfcAxis2Placement3D( swept_surface_placement,  swept_surface_transform );
			}

			shared_ptr<IFC4X3::IfcSurfaceOfLinearExtrusion> linear_extrusion = dynamic_pointer_cast<IFC4X3::IfcSurfaceOfLinearExtrusion>( swept_surface );
			if( linear_extrusion )
			{
				//shared_ptr<IfcDirection>& linear_extrusion_direction = linear_extrusion->m_ExtrudedDirection;
				//shared_ptr<IfcLengthMeasure>& linear_extrusion_depth = linear_extrusion->m_Depth;
				// TODO: implement
				return;
			}

			shared_ptr<IFC4X3::IfcSurfaceOfRevolution> suface_of_revolution = dynamic_pointer_cast<IFC4X3::IfcSurfaceOfRevolution>( swept_surface );
			if( suface_of_revolution )
			{
				// TODO: implement
				return;
			}

			throw UnhandledRepresentationException( surface );
		}
		throw UnhandledRepresentationException( surface );
	}

	void FaceConverter::convertIfcFaceList( const std::vector<shared_ptr<IFC4X3::IfcFace> >& vec_faces, shared_ptr<ItemShapeData> item_data, ShellType st )
	{
		if( vec_faces.size() == 0 )
		{
			return;
		}
		double CARVE_EPSILON = m_geom_settings->getEpsilonCoplanarDistance();
		PolyInputCache3D poly_cache(CARVE_EPSILON);
		GeomProcessingParams params( m_geom_settings, nullptr,  this );
		BuildingEntity* report_entity = nullptr;
		for( const shared_ptr<IFC4X3::IfcFace>& ifc_face : vec_faces )
		{
			if( !ifc_face )
			{
				continue;
			}
			const std::vector<shared_ptr<IFC4X3::IfcFaceBound> >& vec_bounds = ifc_face->m_Bounds;
			std::vector<std::vector<vec3> > face_loops;
			report_entity = ifc_face.get();

			for( auto it_bounds = vec_bounds.begin(); it_bounds != vec_bounds.end(); ++it_bounds )
			{
				const shared_ptr<IFC4X3::IfcFaceBound>& face_bound = ( *it_bounds );

				if( !face_bound )
				{
					continue;
				}

				// ENTITY IfcLoop SUPERTYPE OF(ONEOF(IfcEdgeLoop, IfcPolyLoop, IfcVertexLoop))
				const shared_ptr<IFC4X3::IfcLoop>& loop = face_bound->m_Bound;
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
			try
			{
				item_data->addClosedPolyhedron(poly_cache.m_poly_data, params);
			}
			catch( BuildingException& e )
			{
				// not a fatal error, just mesh is not closed
				messageCallback( e.what(), StatusCallback::MESSAGE_TYPE_MINOR_WARNING, "", report_entity );  // calling function already in e.what()

			}
		}
	}
};
