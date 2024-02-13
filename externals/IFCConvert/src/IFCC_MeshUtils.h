#ifndef IFCC_MeshUtilsH
#define IFCC_MeshUtilsH

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

#include <ifcpp/geometry/IncludeCarveHeaders.h>
#include <ifcpp/model/StatusCallback.h>
#include <earcut/include/mapbox/earcut.hpp>
#include <manifold/src/manifold/include/manifold.h>

#include "IFCC_GeomUtils.h"
#include "IFCC_GeometrySettings.h"
#include "IFCC_GeometryInputData.h"

namespace IFCC {

struct MeshSetInfo
{
	MeshSetInfo()
	{}

	MeshSetInfo( MeshSetInfo& other );

	MeshSetInfo(StatusCallback* _report_callback, BuildingEntity* _entity );

	void resetInfoResult();

	size_t numClosedEdges = 0;
	size_t numOpenEdges = 0;
	size_t numZeroAreaFaces = 0;
	size_t numFaces = 0;
	double surfaceArea = 0;
	bool allPointersValid = true;
	bool meshSetValid = false;
	std::string details;
	StatusCallback* report_callback = nullptr;
	BuildingEntity* entity = nullptr;
};

namespace MeshUtils
{
	size_t getNumFaces( const carve::mesh::MeshSet<3>* meshset );

	size_t getNumClosedEdges( const carve::mesh::MeshSet<3>* meshset );

	double getMeshVolume( const carve::mesh::MeshSet<3>* meshset );

	void roundVertices( carve::mesh::MeshSet<3>* meshset );

	void recalcMeshSet(shared_ptr<carve::mesh::MeshSet<3> >& meshset, double CARVE_EPSILON);

	bool checkMeshSetVolume( const shared_ptr<carve::mesh::MeshSet<3> >& mesh_set, StatusCallback* report_callback,
							 BuildingEntity* entity, double CARVE_EPSILON );


	bool checkEdgePointers(carve::mesh::Edge<3>* e, bool checkForDegenerateEdges, MeshSetInfo& info);

	bool checkMeshFins(const shared_ptr<carve::mesh::MeshSet<3> >& meshset, double CARVE_EPSILON);

	bool checkFaceIntegrity(const carve::mesh::Face<3>* face, bool checkForDegenerateEdges, MeshSetInfo& info);

	bool hasPointer(const carve::mesh::Face<3>* face, const carve::mesh::Edge<3>* edgePtr);

	void countReferencesToVertex(const carve::mesh::Edge<3>* edge, const carve::mesh::Vertex<3>* vertex,
								 std::set<const carve::mesh::Edge<3>* >& setEdgesReferencing);

	bool checkMeshPointers(const carve::mesh::Mesh<3>* mesh,  bool checkForDegenerateEdges, MeshSetInfo& info);

	bool checkMeshSetPointers(const shared_ptr<carve::mesh::MeshSet<3> >& meshset,  bool checkForDegenerateEdges,
							  MeshSetInfo& info);

	void removeFinEdges(const carve::mesh::Face<3>* face, size_t& numChanges, double eps);

	void removeFinEdges(carve::mesh::Mesh<3>* mesh, double CARVE_EPSILON);

	double getFaceArea2D(const carve::mesh::Face<3>* face );

	void checkFaceLoops(const carve::mesh::Face<3>* face);

	void checkFaceLoops( carve::mesh::Mesh<3>* mesh);

	void checkFaceLoops(carve::mesh::MeshSet<3>* meshset);

	void fixMeshset(carve::mesh::MeshSet<3>* meshset, double CARVE_EPSILON, bool shouldBeClosedManifold, bool dumpMeshes);

	bool edgeToEdgeIntersect(const carve::mesh::Edge<3>* edge1, const carve::mesh::Edge<3>* edge2, double eps,
							 vec3& intersectionPoint);

	void createTriangulated3DFace(const std::vector<std::vector<vec3> >& inputBounds3D, PolyInputCache3D& meshOut, GeomProcessingParams& params );

	void create3DFace(const std::vector<std::vector<vec3> >& inputBounds3D, PolyInputCache3D& meshOut, GeomProcessingParams& params );

	///\brief method intersectOpenEdges: Intersect open edges of MeshSet with closed edges, and split the open edges in case of intersection
	///\param[in/out] meshset: MeshSet with open edges. If fix is found, a new MeshSet is assigned to the smart pointer
	///\param[in] eps: tolerance to find edge-edge intersections
	///\param[in] dumpMeshes: write meshes to dump file for debugging
	void intersectOpenEdges(shared_ptr<carve::mesh::MeshSet<3> >& meshset, GeomProcessingParams& params);


	void mesh2MeshSet(const carve::mesh::Mesh<3>* mesh, shared_ptr<carve::mesh::MeshSet<3> >& meshsetResult,
					  double CARVE_EPSILON);


	void polyhedronFromMeshSet(const shared_ptr<carve::mesh::MeshSet<3>>& meshset, PolyInputCache3D& polyInput);

	void polyhedronFromMesh(const carve::mesh::Mesh<3>* mesh, PolyInputCache3D& polyInput);

	bool addFacesReversed(const PolyInputCache3D& poly_cache_source, PolyInputCache3D& poly_cache_target);

	void resolveOpenEdges(shared_ptr<carve::mesh::MeshSet<3>>& meshset, double eps, bool dumpPolygons);

	void intersectOpenEdges(shared_ptr<carve::mesh::MeshSet<3> >& meshset, GeomProcessingParams& params);

	void polyhedronFromMesh(const carve::mesh::Mesh<3>* mesh, PolyInputCache3D& polyInput);

	bool addFacesReversed(const PolyInputCache3D& poly_cache_source, PolyInputCache3D& poly_cache_target);

	bool checkMeshSetValidAndClosed(const shared_ptr<carve::mesh::MeshSet<3>>& meshset, MeshSetInfo& info, double CARVE_EPSILON );

	bool checkMeshSetValidAndClosed(const shared_ptr<carve::mesh::MeshSet<3>>& meshset, MeshSetInfo& info, double eps, bool tryToFixIfNotValid, bool dumpMeshes);

	double computeFaceArea(const carve::mesh::Face<3>* face);

	void getFacesInMeshSet(shared_ptr<carve::mesh::MeshSet<3> >& meshsetInput, std::set<carve::mesh::Face<3>* >& mapAllFaces);

	bool checkMeshSetNonNegativeAndClosed(const shared_ptr<carve::mesh::MeshSet<3>> mesh_set, double CARVE_EPSILON);

	bool checkMeshsetTriangulated(const shared_ptr<carve::mesh::MeshSet<3>>& meshset);

	void removeZeroAreaFacesInMesh(carve::mesh::Mesh<3>* mesh, double epsMinFaceArea, double CARVE_EPSILON);

	void removeZeroAreaFacesInMeshSet(shared_ptr<carve::mesh::MeshSet<3> >& meshsetInput, double epsMinFaceArea, double CARVE_EPSILON);

} // end namespace MeshUtils

} // end namespace IFCC

#endif // IFCC_MeshUtilsH
