#ifndef IFCC_CurveConverterH
#define IFCC_CurveConverterH

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


#include <ifcpp/geometry/GeometrySettings.h>

#include <ifcpp/geometry/Carve/PointConverter.h>
#include <ifcpp/geometry/Carve/SplineConverter.h>

#include "IFCC_PlacementConverter.h"

namespace IFCC {


//\brief class to convert different types of IFC curve representations into carve input geometry
class CurveConverter : public StatusCallback
{
protected:
	shared_ptr<GeometrySettings>	m_geom_settings;
	shared_ptr<PlacementConverter>	m_placement_converter;
	shared_ptr<PointConverter>		m_point_converter;
	shared_ptr<SplineConverter>		m_spline_converter;

public:
	CurveConverter( shared_ptr<GeometrySettings>& gs, shared_ptr<PlacementConverter>& placement_converter,
					shared_ptr<PointConverter>& pc, shared_ptr<SplineConverter>& sc );

	virtual ~CurveConverter() = default;

	const shared_ptr<GeometrySettings>&		getGeomSettings() { return m_geom_settings; }
	const shared_ptr<PlacementConverter>&	getPlacementConverter() { return m_placement_converter; }
	const shared_ptr<PointConverter>&		getPointConverter() { return m_point_converter; }
	const shared_ptr<SplineConverter>&		getSplineConverter() { return m_spline_converter; }

	void convertIfcCurve2D( const shared_ptr<IfcCurve>& ifc_curve, std::vector<vec2>& loops, std::vector<vec2>& segment_start_points ) const;

	void convertIfcCurve2D( const shared_ptr<IfcCurve>& ifc_curve, std::vector<vec2>& target_vec, std::vector<vec2>& segment_start_points,
		std::vector<shared_ptr<IfcTrimmingSelect> >& trim1_vec, std::vector<shared_ptr<IfcTrimmingSelect> >& trim2_vec, bool sense_agreement ) const;

	void convertIfcCurve( const shared_ptr<IfcCurve>& ifc_curve, std::vector<vec3>& loops, std::vector<vec3>& segment_start_points ) const;

	void convertIfcCurve( const shared_ptr<IfcCurve>& ifc_curve, std::vector<vec3>& target_vec, std::vector<vec3>& segment_start_points,
		std::vector<shared_ptr<IfcTrimmingSelect> >& trim1_vec, std::vector<shared_ptr<IfcTrimmingSelect> >& trim2_vec, bool sense_agreement ) const;


	void convertIfcPolyline( const shared_ptr<IfcPolyline>& poly_line, std::vector<vec3>& loop ) const;

	void convertIfcLoop( const shared_ptr<IfcLoop>& loop, std::vector<vec3>& loop_points ) const;
};


} // end namespace

#endif // IFCC_CurveConverterH
