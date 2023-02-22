#ifndef IFCC_SolidModelConverterH
#define IFCC_SolidModelConverterH

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

#include <ifcpp/geometry/GeomUtils.h>

#include <ifcpp/IFC4X3/include/IfcBooleanResult.h>
#include <ifcpp/IFC4X3/include/IfcCsgPrimitive3D.h>
#include <ifcpp/IFC4X3/include/IfcExtrudedAreaSolid.h>
#include <ifcpp/IFC4X3/include/IfcHalfSpaceSolid.h>
#include <ifcpp/IFC4X3/include/IfcRevolvedAreaSolid.h>
#include <ifcpp/IFC4X3/include/IfcSectionedSpine.h>
#include <ifcpp/IFC4X3/include/IfcSolidModel.h>

#include "IFCC_PointConverter.h"
#include "IFCC_ProfileCache.h"
#include "IFCC_FaceConverter.h"
#include "IFCC_CurveConverter.h"

namespace IFCC {

/*! Contains geometry converters for solid geometries.*/
class SolidModelConverter : public StatusCallback
{
public:
	shared_ptr<GeometrySettings>		m_geom_settings;
	shared_ptr<PointConverter>			m_point_converter;
	shared_ptr<CurveConverter>			m_curve_converter;
	shared_ptr<FaceConverter>			m_face_converter;
	shared_ptr<ProfileCache>			m_profile_cache;
	shared_ptr<Sweeper>					m_sweeper;

	SolidModelConverter( shared_ptr<GeometrySettings>& gs, shared_ptr<PointConverter>&	pc, shared_ptr<CurveConverter>& cc,
		shared_ptr<FaceConverter>& fc, shared_ptr<ProfileCache>& pcache, shared_ptr<Sweeper>& sw );

	virtual ~SolidModelConverter();

	// ENTITY IfcSolidModel ABSTRACT SUPERTYPE OF(ONEOF(IfcCsgSolid, IfcManifoldSolidBrep, IfcSweptAreaSolid, IfcSweptDiskSolid))
	void convertIfcSolidModel( const shared_ptr<IFC4X3::IfcSolidModel>& solid_model, shared_ptr<ItemShapeData> item_data );

	void convertIfcExtrudedAreaSolid( const shared_ptr<IFC4X3::IfcExtrudedAreaSolid>& extruded_area, shared_ptr<ItemShapeData> item_data );

	void convertRevolvedAreaSolid( const std::vector<std::vector<vec2> >& profile_coords_unchecked, const vec3& axis_location, const vec3& axis_direction, double revolution_angle, shared_ptr<ItemShapeData> item_data, BuildingEntity* entity_of_origin = nullptr );

	void convertIfcRevolvedAreaSolid( const shared_ptr<IFC4X3::IfcRevolvedAreaSolid>& revolved_area, shared_ptr<ItemShapeData> item_data );

	void convertIfcBooleanResult( const shared_ptr<IFC4X3::IfcBooleanResult>& bool_result, shared_ptr<ItemShapeData> item_data );

	void convertIfcCsgPrimitive3D( const shared_ptr<IFC4X3::IfcCsgPrimitive3D>& csg_primitive, shared_ptr<ItemShapeData> item_data );

	void extrudeBox( const std::vector<vec3>& boundary_points, const vec3& extrusion_vector, shared_ptr<carve::input::PolyhedronData>& box_data );

	void convertIfcHalfSpaceSolid( const shared_ptr<IFC4X3::IfcHalfSpaceSolid>& half_space_solid, shared_ptr<ItemShapeData> item_data, const shared_ptr<ItemShapeData>& other_operand );

	void convertIfcBooleanOperand( const shared_ptr<IFC4X3::IfcBooleanOperand>& operand_select, shared_ptr<ItemShapeData> item_data, const shared_ptr<ItemShapeData>& other_operand );

	void convertIfcSectionedSpine( const shared_ptr<IFC4X3::IfcSectionedSpine>& spine, shared_ptr<ItemShapeData> item_data );
};

} // end namespace

#endif // IFCC_SolidModelConverterH
