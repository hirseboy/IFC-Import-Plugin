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
#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/StatusCallback.h>

#include <ifcpp/IFC4X3/include/IfcBSplineCurve.h>
#include <ifcpp/IFC4X3/include/IfcBSplineSurface.h>

#include "IFCC_GeometrySettings.h"
#include "IFCC_PointConverter.h"

namespace IFCC {


class SplineConverter : public StatusCallback
{
protected:
	shared_ptr<GeometrySettings> m_geom_settings;
	shared_ptr<PointConverter> m_point_converter;

public:
	static void computeKnotVector( const size_t numControlPoints, const size_t order, std::vector<double>& knotVector );

	static void computRationalBasisFunctions( const size_t order, const double t, const size_t numControlPoints, const std::vector<double>& knotVec,
		std::vector<double>& weights, std::vector<double>& basisFunc );

	static void computeRationalBSpline( const size_t order, const size_t numCurvePoints, const std::vector<vec3>& controlPoints, std::vector<double>& weights,
		std::vector<double>& knotVec, std::vector<double>& curvePoints );

	SplineConverter( shared_ptr<GeometrySettings>& geom_settings, shared_ptr<PointConverter>& pt_converter )
		: m_geom_settings( geom_settings ), m_point_converter( pt_converter )
	{
	}

	virtual ~SplineConverter()
	{
	}

	void convertBSplineCurve( const shared_ptr<IFC4X3::IfcBSplineCurve>& bspline_curve, std::vector<vec3>& target_vec,
							  std::vector<vec3>& segment_start_points ) const;

	void convertIfcBSplineSurface( const shared_ptr<IFC4X3::IfcBSplineSurface>& ifc_bspline_surface,
								   shared_ptr<carve::input::PolylineSetData>& polyline_data );
};

}
