#ifndef IFCC_PlacementConverterH
#define IFCC_PlacementConverterH

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

#include <set>
#include <unordered_set>

#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/UnitConverter.h>
#include <ifcpp/IFC4/include/IfcAxis2Placement2D.h>
#include <ifcpp/IFC4/include/IfcAxis2Placement3D.h>
#include <ifcpp/IFC4/include/IfcCartesianTransformationOperator.h>

#include <ifcpp/geometry/Carve/GeomUtils.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>
#include <ifcpp/geometry/Carve/IncludeCarveHeaders.h>

namespace IFCC {

/*! Class for converting axis placements from local to global system.*/
class PlacementConverter : public StatusCallback
{
public:
	shared_ptr<UnitConverter>	m_unit_converter;

	PlacementConverter( shared_ptr<UnitConverter>& uc );

	void convertIfcAxis2Placement2D( const shared_ptr<IfcAxis2Placement2D>& axis2placement2d, shared_ptr<TransformData>& resulting_matrix, bool only_rotation = false );

	void convertIfcAxis2Placement3D( const shared_ptr<IfcAxis2Placement3D>& axis2placement3d, shared_ptr<TransformData>& resulting_matrix, bool only_rotation = false );

	void getPlane( const shared_ptr<IfcAxis2Placement3D>& axis2placement3d, carve::geom::plane<3>& plane, vec3& translate );

	void convertMatrix( const carve::math::Matrix& matrix, shared_ptr<IfcAxis2Placement3D>& axis2placement3d, int& entity_id, std::vector<shared_ptr<BuildingEntity> >& vec_new_entities );

	void convertIfcPlacement( const shared_ptr<IfcPlacement>& placement, shared_ptr<TransformData>& resulting_matrix, bool only_rotation = false );

	void getWorldCoordinateSystem( const shared_ptr<IfcRepresentationContext>& context, shared_ptr<TransformData>& resulting_matrix, std::unordered_set<IfcRepresentationContext*>& already_applied );

	/*! \brief translates an IfcObjectPlacement (or subtype) to carve Matrix.*/
	void convertIfcObjectPlacement( const shared_ptr<IfcObjectPlacement>& ifc_object_placement, shared_ptr<ProductShapeData>& product_data,
		std::unordered_set<IfcObjectPlacement*>& placement_already_applied, bool only_rotation );

	void convertTransformationOperator( const shared_ptr<IfcCartesianTransformationOperator>& transform_operator, shared_ptr<TransformData>& resulting_matrix );
};

} // end namespace

#endif // IFCC_PlacementConverterH
