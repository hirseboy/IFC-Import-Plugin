#ifndef IFCC_ProfileCacheH
#define IFCC_ProfileCacheH

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

#include <ifcpp/IFC4X3/include/IfcCartesianPoint.h>

#include "IFCC_SplineConverter.h"
#include "IFCC_ProfileConverter.h"
#include "IFCC_CurveConverter.h"

namespace IFCC {


class ProfileCache : public StatusCallback
{
protected:
	shared_ptr<CurveConverter>					m_curve_converter;
	shared_ptr<SplineConverter>					m_spline_converter;
	std::map<int,shared_ptr<ProfileConverter> >	m_profile_cache;

#ifdef ENABLE_OPENMP
	Mutex m_writelock_profile_cache;
#endif

public:
	ProfileCache( shared_ptr<CurveConverter>& cc, shared_ptr<SplineConverter>& sc );

	virtual ~ProfileCache() = default;

	void clearProfileCache();

	shared_ptr<ProfileConverter> getProfileConverter( shared_ptr<IFC4X3::IfcProfileDef>& ifc_profile );
};

} // end namespace

#endif // IFCC_CurveConverterH
