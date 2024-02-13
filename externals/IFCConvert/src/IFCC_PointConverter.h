#ifndef IFCC_PointConverterH
#define IFCC_PointConverterH

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

#define _USE_MATH_DEFINES
#include <math.h>

#include <ifcpp/model/BuildingObject.h>
#include <ifcpp/geometry/IncludeCarveHeaders.h>

#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/StatusCallback.h>
#include <ifcpp/model/UnitConverter.h>

#include <ifcpp/IFC4X3/include/IfcCartesianPoint.h>
#include <ifcpp/IFC4X3/include/IfcLengthMeasure.h>
#include <ifcpp/IFC4X3/include/IfcVertex.h>

#include "IFCC_GeometrySettings.h"

namespace IFCC {

//\brief class to convert IFC point representations into carve input geometry
class PointConverter : public StatusCallback
{
protected:
	shared_ptr<UnitConverter>		m_unit_converter;

public:
	PointConverter( shared_ptr<UnitConverter>& uc ): m_unit_converter( uc )
	{
	}

	virtual ~PointConverter(){}

	const shared_ptr<UnitConverter>& getUnitConverter() { return m_unit_converter; }
	void setUnitConverter( shared_ptr<UnitConverter>& unit_converter );

	static bool convertIfcCartesianPoint( const shared_ptr<IFC4X3::IfcCartesianPoint>& ifc_point, vec3& point, double length_factor );

	void convertIfcCartesianPointVector( const std::vector<shared_ptr<IFC4X3::IfcCartesianPoint> >& points, std::vector<vec3>& loop ) const;

	void convertIfcCartesianPointVector2D( const std::vector<std::vector<shared_ptr<IFC4X3::IfcCartesianPoint> > >& vec_points_in,
										   std::vector<vec3>& vertices );

	void convertIfcCartesianPointVectorSkipDuplicates( const std::vector<shared_ptr<IFC4X3::IfcCartesianPoint> >& vec_ifc_points,
													   std::vector<vec3>& loop ) const;

	void convertPointList(const std::vector<std::vector<shared_ptr<IFC4X3::IfcLengthMeasure> > >& pointList, std::vector<vec3>& loop);

	static bool convertIfcVertex(const shared_ptr<IFC4X3::IfcVertex>& vertex, vec3& point_result, const double length_factor);

	static double trimPointCircleDistance(double angle, double radius, const carve::math::Matrix& circlePosition, const vec3& trimPoint);

	//\brief: returns the corresponding angle (radian, 0 is to the right) if the given point lies on the circle. If the point does not lie on the circle, -1 is returned.
	static double getAngleOnCircle( const vec3& circleCenter, double radius, vec3& trimPoint, const carve::math::Matrix& circlePosition,
									const carve::math::Matrix& circlePositionInverse );
};

} // end namespace

#endif // IFCC_PointConverterH
