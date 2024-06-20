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
//#include <ifcpp/geometry/GeomDebugDump.h>
#include <ifcpp/geometry/GeometrySettings.h>
#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/BuildingException.h>
#include <ifcpp/model/StatusCallback.h>

#include <ifcpp/geometry/IncludeCarveHeaders.h>
#include <ifcpp/geometry/PointProjector.h>

#include "IFCC_MeshUtils.h"
#include "IFCC_GeometryInputData.h"

//#include "MeshOpsDebug.h"

namespace IFCC {


struct AdjacentFacePair {
	AdjacentFacePair( carve::mesh::Face<3>* _faceOnEdge, carve::mesh::Face<3>* _faceOnReverseEdge, carve::mesh::Edge<3>* _edge )
		: faceOnEdge(_faceOnEdge), faceOnReverseEdge(_faceOnReverseEdge), edge( _edge)
	{

	}
	bool edgeDeleted = false;
	carve::mesh::Face<3>* faceOnEdge;
	carve::mesh::Face<3>* faceOnReverseEdge;
	carve::mesh::Edge<3>* edge;
};

class CoplanarFaceContainer {
public:
	CoplanarFaceContainer() {}

	PointProjector m_pointProjector;
	carve::mesh::Face<3>* m_face = nullptr;

	double distancePointPlane(const glm::dvec3& point) const
	{
		return m_pointProjector.m_plane.distancePointPlane(point);
	}
	double distancePointPlane(const carve::geom::vector<3>& point) const
	{
		glm::dvec3 point_glm(point.x, point.y, point.z);
		return m_pointProjector.m_plane.distancePointPlane(point_glm);
	}
};

namespace MeshOps {

struct CarveMeshNormalizer
{
	CarveMeshNormalizer( bool normalizeCoordsInsteadOfEpsilon );

	CarveMeshNormalizer(const shared_ptr<carve::mesh::MeshSet<3>>& mesh1, const shared_ptr<carve::mesh::MeshSet<3> >& mesh2, bool normalizeCoordsInsteadOfEpsilon);

	double getScale() const { return m_scale; }
	void setToZero();

	void normalizeMesh(shared_ptr<carve::mesh::MeshSet<3> >& meshset, std::string tag, double CARVE_EPSILON);

	void deNormalizeMesh(shared_ptr<carve::mesh::MeshSet<3> >& meshset, std::string tag, double CARVE_EPSILON);

	bool disableNormalizeAll = false;

private:
	double m_scale = 1.0;
	vec3 m_normalizeCenter;
	std::map<std::string, carve::mesh::MeshSet<3>* > m_normalizedMeshes;
	bool m_normalizeCoordsInsteadOfEpsilon = false;
};

/// \brief simplifyMeshSet merge coplanar faces and re-triangulate each set of merged faces
/// \param meshset				Carve meshset
/// \param entity				IFC entity that is currently being processed
/// \param ignoreOpenEdgesInResult	If true, the result is kept even with open edges (good for visualization). If false, the result will be the input mesh in case open edges occur after triangulation (good for further boolean operations)
void simplifyMeshSet( shared_ptr<carve::mesh::MeshSet<3> >& meshset, shared_ptr<GeometrySettings>& geomSettings,
					  bool triangulateResult, bool shouldBeClosedManifold, bool dumpPolygon, double CARVE_EPSILON);

void simplifyMeshSet(std::vector<shared_ptr<carve::mesh::MeshSet<3>> >& meshsets, shared_ptr<GeometrySettings>& geomSettings,
					 bool triangulateResult, bool shouldBeClosedManifold,
					 bool dumpPolygon, double CARVE_EPSILON);

void retriangulateMeshSetSimple( shared_ptr<carve::mesh::MeshSet<3> >& meshset, bool ignoreResultOpenEdges, double CARVE_EPSILON,
								 size_t retryCount );

size_t mergeCoplanarFacesInMeshSet( shared_ptr<carve::mesh::MeshSet<3> >& meshset, shared_ptr<GeometrySettings>& geomSettings,
									bool shouldBeClosedManifold, bool dumpFaces, double CARVE_EPSILON);

} // end namespace MeshOps

} // end namespace IFCC
