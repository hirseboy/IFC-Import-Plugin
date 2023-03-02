#ifndef IFCC_FaceConverterH
#define IFCC_FaceConverterH

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

#include <ifcpp/model/BuildingObject.h>
#include <ifcpp/geometry/GeomUtils.h>
#include <ifcpp/geometry/GeometrySettings.h>
#include <ifcpp/model/StatusCallback.h>
#include <ifcpp/model/UnitConverter.h>

#include <ifcpp/IFC4X3/include/IfcFace.h>
#include <ifcpp/IFC4X3/include/IfcCartesianPoint.h>
#include <ifcpp/IFC4X3/EntityFactory.h>

#include "IFCC_Sweeper.h"
#include "IFCC_SplineConverter.h"
#include "IFCC_CurveConverter.h"

namespace IFCC {

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

	FaceConverter( shared_ptr<GeometrySettings>& gs, shared_ptr<UnitConverter>& uc, shared_ptr<CurveConverter>& cc,
				   shared_ptr<SplineConverter>& sc, shared_ptr<Sweeper>& sw );

	virtual ~FaceConverter() = default;

	void convertIfcSurface( const shared_ptr<IFC4X3::IfcSurface>& surface, shared_ptr<ItemShapeData>& item_data, shared_ptr<SurfaceProxy>& surface_proxy );

	void convertIfcFaceList( const std::vector<shared_ptr<IFC4X3::IfcFace> >& vec_faces, shared_ptr<ItemShapeData> item_data, ShellType st );
};

} // end namespace

#endif // IFCC_FaceConverterH


