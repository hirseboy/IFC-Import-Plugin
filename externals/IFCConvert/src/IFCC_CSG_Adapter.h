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

#include <ifcpp/geometry/GeometryException.h>
#include <ifcpp/geometry/GeomDebugDump.h>
#include <ifcpp/geometry/GeometrySettings.h>
#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/BuildingException.h>
#include <ifcpp/model/StatusCallback.h>

#include <ifcpp/geometry/IncludeCarveHeaders.h>

#include "IFCC_GeometryInputData.h"

#ifdef _USE_MANIFOLD_CSG
#include "CSG_Manifold.h"
#endif

#define _USE_CLASSIFY_NORMAL

namespace IFCC {

namespace CSG_Adapter
{
	void assignResultOnFail(const shared_ptr<carve::mesh::MeshSet<3> >& op1, const shared_ptr<carve::mesh::MeshSet<3> >& op2,
								   const carve::csg::CSG::OP operation, shared_ptr<carve::mesh::MeshSet<3> >& result);

	bool checkBoundinbBoxIntersection(const shared_ptr<carve::mesh::MeshSet<3> >& op1, const shared_ptr<carve::mesh::MeshSet<3> >& op2,
											 const carve::csg::CSG::OP operation);

	bool computeCSG_Carve(const shared_ptr<carve::mesh::MeshSet<3> >& op1Orig, const shared_ptr<carve::mesh::MeshSet<3> >& op2Orig, const carve::csg::CSG::OP operation, shared_ptr<carve::mesh::MeshSet<3> >& result,
		shared_ptr<GeometrySettings>& geom_settings, StatusCallback* report_callback, const shared_ptr<BuildingEntity>& entity, bool normalizeCoords);

	void computeCSG(shared_ptr<carve::mesh::MeshSet<3> >& op1, std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& operands2, const carve::csg::CSG::OP operation, shared_ptr<carve::mesh::MeshSet<3> >& result,
		shared_ptr<GeometrySettings>& geom_settings, StatusCallback* report_callback, const shared_ptr<BuildingEntity>& entity);

} // end namespace CSG_Adapter

} // end namespace IFCC
