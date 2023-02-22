#ifndef IFCC_ProfileConverterH
#define IFCC_ProfileConverterH

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

#include <ifcpp/model/StatusCallback.h>
#include <ifcpp/IFC4X3/include/IfcArbitraryClosedProfileDef.h>
#include <ifcpp/IFC4X3/include/IfcArbitraryOpenProfileDef.h>
#include <ifcpp/IFC4X3/include/IfcCompositeProfileDef.h>
#include <ifcpp/IFC4X3/include/IfcDerivedProfileDef.h>
#include <ifcpp/IFC4X3/include/IfcParameterizedProfileDef.h>
#include <ifcpp/IFC4X3/include/IfcProfileDef.h>

#include "IFCC_CurveConverter.h"

namespace IFCC {

class ProfileConverter : public StatusCallback
{
public:

	const std::vector<std::vector<vec2> >& getCoordinates() { return m_paths; }
	void clearProfileConverter() { m_paths.clear(); }

	shared_ptr<CurveConverter>				m_curve_converter;
	shared_ptr<SplineConverter>				m_spline_converter;

protected:
	std::vector<std::vector<vec2> >	m_paths;

public:
	ProfileConverter( shared_ptr<CurveConverter>& cc, shared_ptr<SplineConverter>& sc );

	virtual ~ProfileConverter() = default;

	void computeProfile( shared_ptr<IFC4X3::IfcProfileDef> profile_def );

	void addAvoidingDuplicates( const std::vector<vec2>& polygon, std::vector<std::vector<vec2> >& paths );

	void convertIfcArbitraryClosedProfileDef( const shared_ptr<IFC4X3::IfcArbitraryClosedProfileDef>& profile, std::vector<std::vector<vec2> >& paths );

	void convertIfcArbitraryOpenProfileDef( const shared_ptr<IFC4X3::IfcArbitraryOpenProfileDef>& profile, std::vector<std::vector<vec2> >& paths );

	void convertIfcCompositeProfileDef( const shared_ptr<IFC4X3::IfcCompositeProfileDef>& composite_profile, std::vector<std::vector<vec2> >& paths );

	void convertIfcDerivedProfileDef( const shared_ptr<IFC4X3::IfcDerivedProfileDef>& derived_profile, std::vector<std::vector<vec2> >& paths );

	void convertIfcParameterizedProfileDefWithPosition( const shared_ptr<IFC4X3::IfcParameterizedProfileDef>& parameterized, std::vector<std::vector<vec2> >& paths );

	void convertIfcParameterizedProfileDef( const shared_ptr<IFC4X3::IfcParameterizedProfileDef>& profile, std::vector<std::vector<vec2> >& paths );

	static void deleteLastPointIfEqualToFirst( std::vector<vec2>& coords );

	void simplifyPaths();

	static void simplifyPaths( std::vector<std::vector<vec2> >& paths );

	static void simplifyPath( std::vector<vec2>& path );

	void addArc( std::vector<vec2>& coords, double radius, double start_angle, double opening_angle, double x_center, double y_center, int num_segments = 4 ) const;

	static void mirrorCopyPath( std::vector<vec2>& coords, bool mirror_on_y_axis, bool mirror_on_x_axis );

	static void mirrorCopyPathReverse( std::vector<vec2>& coords, bool mirror_on_y_axis, bool mirror_on_x_axis );

};


} // end namespace

#endif // IFCC_ProfileConverterH
