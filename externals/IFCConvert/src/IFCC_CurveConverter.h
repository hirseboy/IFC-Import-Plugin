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

#include <ifcpp/model/BuildingObject.h>

//#include <ifcpp/geometry/GeomDebugDump.h>

#include <ifcpp/IFC4X3/include/IfcEdge.h>
#include <ifcpp/IFC4X3/include/IfcPolyline.h>
#include <ifcpp/IFC4X3/include/IfcLoop.h>

#include <ifcpp/IFC4X3/EntityFactory.h>

#include <ifcpp/IFC4X3/include/IfcCartesianPoint.h>

#include <ifcpp/geometry/IncludeCarveHeaders.h>

#include "IFCC_GeomUtils.h"
#include "IFCC_GeometrySettings.h"
#include "IFCC_SplineConverter.h"
#include "IFCC_PointConverter.h"
#include "IFCC_PlacementConverter.h"


namespace IFCC {

//\brief class to convert different types of IFC curve representations into carve input geometry
class CurveConverter : public StatusCallback
{
protected:
	shared_ptr<GeometrySettings>	m_geom_settings;
	shared_ptr<IFCC::PlacementConverter>	m_placement_converter;
	shared_ptr<PointConverter>		m_point_converter;
	shared_ptr<SplineConverter>		m_spline_converter;

public:
	CurveConverter(shared_ptr<GeometrySettings>& gs, shared_ptr<PlacementConverter>& placement_converter,
				   shared_ptr<PointConverter>& pc, shared_ptr<SplineConverter>& sc);

	virtual ~CurveConverter() = default;

	const shared_ptr<GeometrySettings>& getGeomSettings() { return m_geom_settings; }
	const shared_ptr<PlacementConverter>& getPlacementConverter() { return m_placement_converter; }
	const shared_ptr<PointConverter>& getPointConverter() { return m_point_converter; }
	const shared_ptr<SplineConverter>& getSplineConverter() { return m_spline_converter; }

	void convertIfcCurve2D(const shared_ptr<IFC4X3::IfcCurve>& ifc_curve, std::vector<vec2>& loops, std::vector<vec2>& segment_start_points,
						   bool senseAgreement) const;

	void convertIfcCurve2D(const shared_ptr<IFC4X3::IfcCurve>& ifc_curve, std::vector<vec2>& target_vec, std::vector<vec2>& segment_start_points,
		std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim1_vec, std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim2_vec, bool senseAgreement) const;

	void convertIfcCurve(const shared_ptr<IFC4X3::IfcCurve>& ifc_curve, std::vector<vec3>& loops, std::vector<vec3>& segment_start_points,
						 bool senseAgreement) const;

	void convertIfcCurve(const shared_ptr<IFC4X3::IfcCurve>& ifc_curve, std::vector<vec3>& target_vec, std::vector<vec3>& segment_start_points,
		std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim1_vec, std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim2_vec,
						 bool senseAgreement) const;

	void getTrimAngles(const std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim1_vec, const std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim2_vec, vec3 circle_center,
		double circle_radius, bool senseAgreement, double& trimAngle1, double& trimAngle2, double& startAngle,
					   double& openingAngle, const carve::math::Matrix& circlePlacement, const carve::math::Matrix& circlePlacementInverse) const;

	void getTrimPoints(const std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim1_vec, const std::vector<shared_ptr<IFC4X3::IfcTrimmingSelect> >& trim2_vec, const carve::math::Matrix& circlePosition,
		double circleRadius, double circleRadius2, bool senseAgreement, vec3& trimPoint1, vec3& trimPoint2) const;

	void convertIfcPolyline(const shared_ptr<IFC4X3::IfcPolyline>& poly_line, std::vector<vec3>& loop) const;

	void convertIfcEdge(const shared_ptr<IFC4X3::IfcEdge>& edge, std::vector<vec3>& loopPoints, double length_factor ) const;

	void convertIfcLoop( const shared_ptr<IFC4X3::IfcLoop>& loop, std::vector<vec3>& loop_points ) const;};

}
