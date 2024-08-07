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

#include "IFCC_MeshOps.h"

namespace IFCC {

namespace MeshOps
{


//\brief getAdjacentFaces: get all faces that are adjacent to fx, by sharing an edge, or by sharing a vertex
static void getAdjacentFaces(carve::mesh::Face<3>* fx, std::set<carve::mesh::Face<3>* >& adjacentFaces)
{

	carve::mesh::Edge<3>* firstEdge = fx->edge;
	if( !firstEdge )
	{
		return;
	}

	carve::mesh::Mesh<3>* mesh = fx->mesh;
	size_t numFacesInMesh = mesh->faces.size();
	carve::mesh::Edge<3>* firstEdgeRev = firstEdge->rev;
	if( !firstEdgeRev )
	{
		return;
	}
	carve::mesh::Face<3>* firstAdjacentFace = firstEdgeRev->face;
	if( !firstAdjacentFace )
	{
		return;
	}
	adjacentFaces.insert(firstAdjacentFace);

	carve::mesh::Edge<3>* edgeRev = firstEdge->rev;

	for( size_t ii = 0; ii < numFacesInMesh; ++ii )
	{
		carve::mesh::Edge<3>* nextEdgeRev = edgeRev->prev->rev;
		if( !nextEdgeRev )
		{
			return;
		}
		carve::mesh::Face<3>* nextAdjacentFace = nextEdgeRev->face;

		if( nextAdjacentFace )
		{
			if( nextAdjacentFace == fx )
			{
				nextEdgeRev = nextEdgeRev->rev;
				nextAdjacentFace = nextEdgeRev->face;
			}

			if( nextAdjacentFace == firstAdjacentFace )
			{
				break;
			}

			adjacentFaces.insert(nextAdjacentFace);
		}
		else
		{
		}

		edgeRev = nextEdgeRev;
	}

}

static void getFaceEdges(carve::mesh::Face<3>* f, std::set<carve::mesh::Edge<3>* >& faceEdges, bool addReverseFaces )
{
	carve::mesh::Edge<3>* edge = f->edge;
	if( !edge )
	{
		return;
	}

	for( size_t ii = 0; ii < f->n_edges; ++ii )
	{
		faceEdges.insert(edge);

		if( addReverseFaces )
		{
			if( edge->rev )
			{
				faceEdges.insert(edge->rev);
			}
		}
		edge = edge->next;
		if( !edge )
		{
			break;
		}
		if( edge == f->edge )
		{
			break;
		}

	}
}

static void getEdgeLenth2( carve::mesh::Face<3>* f, std::multimap<double, carve::mesh::Edge<3>* >& mapEdgeLenth2)
{
	carve::mesh::Edge<3>* edge = f->edge;
	for( size_t ii = 0; ii < f->n_edges; ++ii )
	{
		double length2 = edge->length2();
		mapEdgeLenth2.insert({ length2, edge });
		edge = edge->next;
	}
}

static size_t replaceVertexPointer(carve::mesh::Vertex<3>* vertexFrom, carve::mesh::Vertex<3>* vertexTo, carve::mesh::Edge<3>* edge)
{
	size_t numChanges = 0;
	if( edge->vert == vertexFrom )
	{
		edge->vert = vertexTo;
		++numChanges;
	}
	if( edge->rev->vert == vertexFrom )
	{
		edge->rev->vert = vertexTo;
		++numChanges;
	}
	return numChanges;
}

static size_t replaceVertexPointer( carve::mesh::Vertex<3>* vertexFrom, carve::mesh::Vertex<3>* vertexTo, std::set<carve::mesh::Face<3>* >& faces)
{
	size_t numChanges = 0;
	for( auto f : faces )
	{
		carve::mesh::Edge<3>* edge = f->edge;

		for( size_t ii = 0; ii < f->n_edges; ++ii )
		{
			numChanges += replaceVertexPointer(vertexFrom, vertexTo, edge);
			edge = edge->next;
			if( edge == f->edge )
			{
				break;
			}
		}
	}
	return numChanges;
}

static size_t replaceFacePointer(carve::mesh::Face<3>* faceFrom, carve::mesh::Face<3>* faceTo, carve::mesh::Edge<3>* edge)
{
	size_t numChanges = 0;
	if( edge )
	{
		if( edge->face == faceFrom )
		{
			edge->face = faceTo;
			++numChanges;
		}
		if( edge->rev )
		{
			if( edge->rev->face == faceFrom )
			{
				edge->rev->face = faceTo;
				++numChanges;
			}
		}
	}
	return numChanges;
}

static size_t replaceFacePointer( carve::mesh::Face<3>* faceFrom, carve::mesh::Face<3>* faceTo, std::set<carve::mesh::Face<3>* >& faces)
{
	size_t numChanges = 0;
	for( auto f : faces )
	{
		if( !f )
		{
			continue;
		}
		carve::mesh::Edge<3>* edge = f->edge;
		if( !edge )
		{
			//std::cout << "invalid edge pointer" << std::endl;
			continue;
		}

		for( size_t ii = 0; ii < f->n_edges; ++ii )
		{
			numChanges += replaceFacePointer(faceFrom, faceTo, edge);
			edge = edge->next;
			if( edge == f->edge )
			{
				break;
			}
		}
	}
	return numChanges;
}

static size_t replaceFacePointer( carve::mesh::Face<3>* faceFrom, carve::mesh::Face<3>* faceTo)
{
	size_t numChanges = 0;
	carve::mesh::Mesh<3>* mesh = faceFrom->mesh;
	std::vector<carve::mesh::Face<3>* >& vecFaces = mesh->faces;
	for( auto f : vecFaces )
	{
		carve::mesh::Edge<3>* edge = f->edge;
		if( !edge )
		{
			continue;
		}

		for( size_t ii = 0; ii < f->n_edges; ++ii )
		{
			numChanges += replaceFacePointer(faceFrom, faceTo, edge);
			edge = edge->next;
			if( edge == f->edge )
			{
				break;
			}
		}
	}
	return numChanges;
}

static size_t countEdges(carve::mesh::Face<3>* face)
{
	if( !face )
	{
		return 0;
	}
	size_t numEdges = 0;
	carve::mesh::Edge<3>* edge = face->edge;
	for( size_t ii = 0; ii < 10000; ++ii )
	{
		++numEdges;
		edge = edge->next;
		if( edge == face->edge )
		{
			break;
		}
	}

	face->n_edges = numEdges;
	return numEdges;
}

static void setEdgeNullPointers(carve::mesh::Edge<3>* edge)
{
	edge->vert = nullptr;
	edge->rev = nullptr;
	edge->face = nullptr;
	edge->prev = nullptr;
	edge->next = nullptr;
}

static size_t removeFaceFromMesh(carve::mesh::Face<3>* fx)
{
	carve::mesh::Mesh<3>* mesh = fx->mesh;
	for( auto it = mesh->faces.begin(); it != mesh->faces.end(); ++it )
	{
		carve::mesh::Face<3>* f = *it;
		if( f == fx )
		{
			mesh->faces.erase(it);
			return 1;
		}
	}
	return 0;
}

static size_t getNumFaces(const shared_ptr<carve::mesh::MeshSet<3> >& meshset )
{
	size_t numFaces = 0;
	for( auto mesh: meshset->meshes)
	{
		numFaces += mesh->faces.size();
	}
	return numFaces;
}

static bool checkMeshsetTriangulated(const shared_ptr<carve::mesh::MeshSet<3>>&meshset)
{
	for( size_t ii = 0; ii < meshset->meshes.size(); ++ii )
	{
		carve::mesh::Mesh<3>* mesh = meshset->meshes[ii];
		const std::vector<carve::mesh::Face<3>*>& vec_faces = mesh->faces;

		for( size_t i2 = 0; i2 < vec_faces.size(); ++i2 )
		{
			carve::mesh::Face<3>* inputFace = vec_faces[i2];
			if( inputFace->n_edges != 3 )
			{
				return false;
			}
		}
	}
	return true;
}

// struct CarveMeshNormalizer

	CarveMeshNormalizer::CarveMeshNormalizer( bool normalizeCoordsInsteadOfEpsilon )
	{
		m_normalizeCoordsInsteadOfEpsilon = normalizeCoordsInsteadOfEpsilon;
	}

	CarveMeshNormalizer::CarveMeshNormalizer(const shared_ptr<carve::mesh::MeshSet<3>>& mesh1, const shared_ptr<carve::mesh::MeshSet<3> >& mesh2, bool normalizeCoordsInsteadOfEpsilon)
	{
		m_normalizeCoordsInsteadOfEpsilon = normalizeCoordsInsteadOfEpsilon;
		if( !mesh1 )
		{
			return;
		}

		carve::geom::aabb<3> bbox1 = mesh1->getAABB();
		vec3 center1 = bbox1.pos;
		vec3 extents1 = bbox1.extent;

		carve::geom::aabb<3> bbox2 = mesh2->getAABB();
		if( bbox2.isEmpty() )
		{
			return;
		}

		vec3 center2 = bbox2.pos;
		vec3 extents2 = bbox2.extent;

		double extent1 = std::max(extents1.x, std::max(extents1.y, extents1.z));
		double extent2 = std::max(extents2.x, std::max(extents2.y, extents2.z));
		double maxExtent = std::max(extent1, extent2);

		double scale = 1.0;
		if( maxExtent > EPS_M5 )
		{
			scale = 1.0 / maxExtent;
		}
		m_scale = scale;

		vec3 center = (center1 + center2) * 0.5;
		m_normalizeCenter = center;

		if( std::abs(m_scale) > 0.5 && std::abs(m_scale) < 2.0 )
		{
			// if scale is close to 1, scaling does not make a big difference.
			// Negative effects like small inaccuracies after multiplication and time to multiply overweigh
			m_scale = 1.0;
		}
	}

	void CarveMeshNormalizer::setToZero()
	{
		m_scale = 1.0;
		m_normalizeCenter.setZero();
	}

	void CarveMeshNormalizer::normalizeMesh(shared_ptr<carve::mesh::MeshSet<3> >& meshset, std::string tag, double CARVE_EPSILON)
	{
		if( disableNormalizeAll )
		{
			return;
		}
		double centerLength2 = m_normalizeCenter.length2();
		if( m_scale == 1.0 && centerLength2 < 2.0 )
		{
			return;
		}

		if( tag.size() > 0 )
		{
			if( m_normalizedMeshes.find(tag) != m_normalizedMeshes.end() )
			{
				// already normalized
				return;
			}
		}

		std::vector<carve::mesh::Vertex<3> >& vertex_storage = meshset->vertex_storage;
		const size_t num_vertices = vertex_storage.size();

		for( size_t i = 0; i < num_vertices; ++i )
		{
			carve::mesh::Vertex<3>& vertex = vertex_storage[i];
			vec3& point = vertex.v;

			if( m_scale != 1.0 && m_normalizeCoordsInsteadOfEpsilon )
			{
				point = (point - m_normalizeCenter) * m_scale;
			}
			else
			{
				point = (point - m_normalizeCenter);
			}
		}

		for( size_t kk = 0; kk < meshset->meshes.size(); ++kk )
		{
			carve::mesh::Mesh<3>* mesh = meshset->meshes[kk];
			mesh->recalc(CARVE_EPSILON);
		}

		m_normalizedMeshes.insert({ tag, meshset.get() });
	}

	void CarveMeshNormalizer::deNormalizeMesh(shared_ptr<carve::mesh::MeshSet<3> >& meshset, std::string tag, double CARVE_EPSILON)
	{
		if( disableNormalizeAll )
		{
			return;
		}
		double centerLength2 = m_normalizeCenter.length2();
		if( m_scale == 1.0 && centerLength2 < 2.0 )
		{
			return;
		}

		if( tag.size() > 0 )
		{
			auto it = m_normalizedMeshes.find(tag);
			if( it == m_normalizedMeshes.end() )
			{
				// already denormalized, or not normalized before
				return;
			}
			m_normalizedMeshes.erase(it);
		}

		double unScaleFactor = (1.0 / m_scale);

		std::vector<carve::mesh::Vertex<3> >& vertex_storage = meshset->vertex_storage;
		const size_t num_vertices = vertex_storage.size();
		for( size_t i = 0; i < num_vertices; ++i )
		{
			carve::mesh::Vertex<3>& vertex = vertex_storage[i];
			vec3& point = vertex.v;
			if( m_scale != 1.0 && m_normalizeCoordsInsteadOfEpsilon )
			{
				point = point * unScaleFactor + m_normalizeCenter;
			}
			else
			{
				point = point + m_normalizeCenter;
			}
		}
		for( size_t kk = 0; kk < meshset->meshes.size(); ++kk )
		{
			carve::mesh::Mesh<3>* mesh = meshset->meshes[kk];
			mesh->recalc(CARVE_EPSILON);
		}
	}


static void splitIntoSubLoops(const std::vector<carve::geom::vector<2> >& polygonMerged, std::vector<std::vector<carve::geom::vector<2> > >& polygonLoops, std::map<size_t, std::array<size_t, 2> >& mapInOut, double eps, bool dumpPolygon )
{
	// find inner loops and shift them such that there is no self-intersection
	std::vector<carve::geom::vector<2>> previousLoop;

	for( size_t ii = 0; ii < polygonMerged.size(); ++ii )
	{
		const carve::geom::vector<2>& point = polygonMerged[ii];


		for( size_t jj = 0; jj < previousLoop.size(); ++jj )
		{
			const carve::geom::vector<2>& previousPoint = previousLoop[jj];
			double dx = previousPoint[0] - point.x;
			if( std::abs(dx) < eps )
			{
				double dy = previousPoint[1] - point.y;
				if( std::abs(dy) < eps )
				{
					// a loop should have at least 3 points
					if( previousLoop.size() < 3 )
					{
						previousLoop.clear();
						break;
					}
					else
					{
						if( jj == 1 )
						{
							// connecting edge between outer and inner loop, erase
							if( previousLoop.size() > 1 )
							{
								previousLoop.erase(previousLoop.begin());
							}
							polygonLoops.push_back(previousLoop);
							previousLoop.clear();
							break;
						}
						// back on point in previous loop. Split here and begin new loop
						polygonLoops.push_back(previousLoop);
						previousLoop.clear();
					}
					break;
				}
			}
		}

		size_t loopIndex = polygonLoops.size();
		size_t pointIndex = previousLoop.size();
		mapInOut[ii] = { loopIndex, pointIndex };
		previousLoop.push_back( point );
	}

	if( previousLoop.size() > 2 )
	{
		polygonLoops.push_back(previousLoop);
	}

	std::multimap<double, std::vector<carve::geom::vector<2> > > mapAreaLoops;
	for( size_t ii = 0; ii < polygonLoops.size(); ++ii )
	{
		std::vector<carve::geom::vector<2> >& loop = polygonLoops[ii];
		double area = std::abs( GeomUtils::signedArea(loop) );
		mapAreaLoops.insert({ area, loop });
	}
	polygonLoops.clear();
	for( auto it = mapAreaLoops.rbegin(); it != mapAreaLoops.rend(); ++it )
	{
		polygonLoops.push_back(it->second);
	}
}

static void shiftSubLoops(std::vector<carve::geom::vector<2> >& polygonMerged, std::map<size_t, size_t >& mapInOut, double eps, bool dumpPolygon )
{
	// find inner loops and shift them such that there is no self-intersection
	std::vector<carve::geom::vector<2>> previousLoop;
	std::vector<std::vector<carve::geom::vector<2> > > polygonLoops;
	for( size_t ii = 0; ii < polygonMerged.size(); ++ii )
	{
		const carve::geom::vector<2>& point = polygonMerged[ii];

		for( size_t jj = 0; jj < previousLoop.size(); ++jj )
		{
			const carve::geom::vector<2>& previousPoint = previousLoop[jj];
			double dx = previousPoint[0] - point.x;
			if( std::abs(dx) < eps )
			{
				double dy = previousPoint[1] - point.y;
				if( std::abs(dy) < eps )
				{
					// a loop should have at least 3 points
					if( previousLoop.size() < 3 )
					{
						previousLoop.clear();
						break;
					}
					else
					{
						if( jj == 1 )
						{
							// connecting edge between outer and inner loop, erase
							if( previousLoop.size() > 1 )
							{
								previousLoop.erase(previousLoop.begin());
							}
							polygonLoops.push_back(previousLoop);
							previousLoop.clear();
							break;
						}
						// back on point in previous loop. Split here and begin new loop
						polygonLoops.push_back(previousLoop);
						previousLoop.clear();
					}
					break;
				}
			}

		}

		size_t loopIndex = polygonLoops.size();
		size_t pointIndex = previousLoop.size();
		//mapInOut[ii] = { loopIndex, pointIndex };
		previousLoop.push_back( point );
	}

	if( previousLoop.size() > 2 )
	{
		//polygon_earcut_out.push_back(previousLoop);
	}

}

void retriangulateMeshSetSimple( shared_ptr<carve::mesh::MeshSet<3> >& meshset, bool ignoreResultOpenEdges, double CARVE_EPSILON, size_t retryCount )
{
	if( !meshset )
	{
		return;
	}

	MeshSetInfo info;
	bool validInput = MeshUtils::checkMeshSetValidAndClosed(meshset, info, CARVE_EPSILON);
	MeshUtils::checkMeshSetNonNegativeAndClosed( meshset, CARVE_EPSILON );

	bool already_triagulated = MeshUtils::checkMeshsetTriangulated(meshset);
	if( already_triagulated )
	{
		return;
	}

	PolyInputCache3D poly_cache( CARVE_EPSILON );  // TODO: check to use double
	std::vector<size_t> map_merged_idx;
	map_merged_idx.resize( meshset->vertex_storage.size(), 0 );
	for( size_t ii = 0; ii < meshset->meshes.size(); ++ii )
	{
		carve::mesh::Mesh<3>* mesh = meshset->meshes[ii];
		std::vector<carve::mesh::Face<3>* >& vec_faces = mesh->faces;

		for( size_t i2 = 0; i2 < vec_faces.size(); ++i2 )
		{
			carve::mesh::Face<3>* face = vec_faces[i2];

			std::vector<vec2> verts2d;
			face->getProjectedVertices( verts2d );
			if( verts2d.size() < 3 )
			{
				continue;
			}

			// now insert points to polygon, avoiding points with same coordinates
			int i_vert = 0;
			carve::mesh::Edge<3>* edge = face->edge;
			for( size_t i3 = 0; i3 < face->n_edges; ++i3 )
			{
				const vec3& v = edge->vert->v;
				edge = edge->next;
				int vertex_index = poly_cache.addPoint(v);
				if( i_vert >= map_merged_idx.size() )
				{
					map_merged_idx.resize(i_vert + 1, 0);
				}
				map_merged_idx[i_vert] = vertex_index;
				++i_vert;
				if( edge == face->edge )
				{
					break;
				}
			}

			if( retryCount > 0 )
			{

				std::map<size_t, size_t> mapFlat2NestedArray;
				if( verts2d.size() > 7 )
				{
					shiftSubLoops(verts2d, mapFlat2NestedArray, CARVE_EPSILON, false);
				}


			}

			{
				std::vector<carve::triangulate::tri_idx> triangulated;
				if( verts2d.size() > 3 )
				{
					try
					{
						carve::triangulate::triangulate(verts2d, triangulated, CARVE_EPSILON);
						carve::triangulate::improve(verts2d, triangulated);
					}
					catch( ... )
					{
						continue;
					}
				}
				else
				{
					triangulated.push_back(carve::triangulate::tri_idx(0, 1, 2));
				}



				for( size_t i4 = 0; i4 != triangulated.size(); ++i4 )
				{
					const carve::triangulate::tri_idx& triangle = triangulated[i4];
					int a = triangle.a;
					int b = triangle.b;
					int c = triangle.c;

					int vertex_id_a = map_merged_idx[a];
					int vertex_id_b = map_merged_idx[b];
					int vertex_id_c = map_merged_idx[c];

					if( vertex_id_a == vertex_id_b || vertex_id_a == vertex_id_c || vertex_id_b == vertex_id_c )
					{
						continue;
					}

					const carve::poly::Vertex<3>& v_a = poly_cache.m_poly_data->getVertex(vertex_id_a);
					const carve::poly::Vertex<3>& v_b = poly_cache.m_poly_data->getVertex(vertex_id_b);

					double dx = v_a.v[0] - v_b.v[0];
					if( std::abs(dx) < 0.0000001 )
					{
						double dy = v_a.v[1] - v_b.v[1];
						if( std::abs(dy) < 0.0000001 )
						{
							double dz = v_a.v[2] - v_b.v[2];
							if( std::abs(dz) < 0.0000001 )
							{
								continue;
							}
						}
					}

					poly_cache.m_poly_data->addFace(vertex_id_a, vertex_id_b, vertex_id_c);
				}
			}
		}
	}

	shared_ptr<carve::mesh::MeshSet<3>> meshsetTrinangulated1 = shared_ptr<carve::mesh::MeshSet<3> >( poly_cache.m_poly_data->createMesh( carve::input::opts(), CARVE_EPSILON ) );

	double minFaceArea = CARVE_EPSILON * 0.01;
	bool correct = checkPolyhedronData(poly_cache.m_poly_data, minFaceArea);
	if( !correct )
	{
		fixPolyhedronData(poly_cache.m_poly_data, minFaceArea);
	}

	shared_ptr<carve::mesh::MeshSet<3>> meshsetTrinangulated = shared_ptr<carve::mesh::MeshSet<3> >( poly_cache.m_poly_data->createMesh( carve::input::opts(), CARVE_EPSILON ) );
	MeshSetInfo infoTriangulated;
	bool validTriangulatedMesh = MeshUtils::checkMeshSetValidAndClosed(meshsetTrinangulated, infoTriangulated, CARVE_EPSILON);
	if( !validTriangulatedMesh )
	{
		bool validTriangulatedMesh1 = MeshUtils::checkMeshSetValidAndClosed(meshsetTrinangulated1, infoTriangulated, CARVE_EPSILON);
		if( validTriangulatedMesh1 )
		{
			meshset.reset();
			meshset = meshsetTrinangulated1;
			return;
		}

		if( !ignoreResultOpenEdges )
		{
			return;
		}
	}
	meshset.reset();
	meshset = meshsetTrinangulated;
}

static void retriangulateMeshSetSimple( std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& meshsets, bool ignoreResultOpenEdges, double eps, size_t retryCount )
{
	for( shared_ptr<carve::mesh::MeshSet<3> >& meshset : meshsets )
	{
		retriangulateMeshSetSimple(meshset, ignoreResultOpenEdges, eps, retryCount);
	}
}

static bool isCoplanar(const shared_ptr<CoplanarFaceContainer>& coplanar, const carve::mesh::Face<3>* faceIn, shared_ptr<GeometrySettings>& geomSettings)
{
	const vec3& faceNormal_carve = faceIn->plane.N;
	const vec3& facePosition_carve = faceIn->edge->v2()->v;

	glm::dvec3 faceNormal(faceNormal_carve.x, faceNormal_carve.y, faceNormal_carve.z);
	glm::dvec3 facePosition(facePosition_carve.x, facePosition_carve.y, facePosition_carve.z);

	PointProjector& pointPro = coplanar->m_pointProjector;
	const glm::dvec3& coplanarNormal = pointPro.m_plane.getNormal();
	const glm::dvec3& coplanarPosition = pointPro.m_plane.getPosition();

	double dotProduct = std::abs(dot(faceNormal, coplanarNormal));
	if( std::abs(dotProduct - 1.0) > geomSettings->getEpsilonCoplanarAngle() )// m_epsCoplanarAngle )  // we can be less strict here, since we test each point separately
	{
		return false;
	}

	std::vector<carve::mesh::Face<3>::vertex_t* > faceVertices;
	faceIn->getVertices(faceVertices);
	if( faceVertices.size() > 2 )
	{
		bool allVerticesInPlane = true;
		for( const carve::mesh::Face<3>::vertex_t * vertex : faceVertices )
		{
			const carve::geom::vector<3>& facePoint_carve = vertex->v;
			glm::dvec3 facePoint(facePoint_carve.x, facePoint_carve.y, facePoint_carve.z);
			double distanceToPlane = pointPro.m_plane.distancePointPlane(facePoint);

			if( std::abs(distanceToPlane) > geomSettings->getEpsilonCoplanarDistance() )// m_epsCoplanarDistance )
			{
				allVerticesInPlane = false;
				break;
			}
		}

		if( allVerticesInPlane )
		{
			return true;
		}
	}
	return false;
}

static bool isCoplanar(const carve::geom::plane<3>& plane, const carve::mesh::Face<3>* face1, const carve::mesh::Face<3>* face2, shared_ptr<GeometrySettings>& geomSettings)
{
	if( !face2 )
	{
		return false;
	}
	const vec3& planeNormal = plane.N;
	const vec3& face2Normal = face2->plane.N;
	const vec3& face2Position = face2->edge->v2()->v;

	double dotProduct = std::abs(dot(planeNormal, face2Normal));
	if( std::abs(dotProduct - 1.0) > geomSettings->getEpsilonCoplanarAngle() )// m_epsCoplanarAngle )  // we can be less strict here, since we test each point separately
	{
		return false;
	}

	//const carve::geom::plane<3>& face1Plane = face1->plane;
	vec3 planePosition = planeNormal;
	planePosition *= -plane.d;

	std::vector<carve::mesh::Face<3>::vertex_t* > faceVertices;
	face2->getVertices(faceVertices);
	if( faceVertices.size() > 2 )
	{
		bool allVerticesInPlane = true;
		for( const carve::mesh::Face<3>::vertex_t * vertex : faceVertices )
		{
			const carve::geom::vector<3>& facePoint = vertex->v;

			double distanceToPlane = std::abs(dot(planeNormal, facePoint - planePosition));

			if( std::abs(distanceToPlane) > geomSettings->getEpsilonCoplanarDistance() )// m_epsCoplanarDistance )
			{
				allVerticesInPlane = false;
				break;
			}
		}

		if( allVerticesInPlane )
		{
			return true;
		}
	}
	return false;
}

static void getAdjacentCoplanarFaces(const carve::geom::plane<3>& plane, carve::mesh::Face<3>* faceIn, std::set<shared_ptr<AdjacentFacePair> >& setAdjacentFaces, shared_ptr<GeometrySettings>& geomSettings, bool recursive)
{
	carve::mesh::Edge<3>* edge = faceIn->edge;
	if( !edge )
	{
		return;
	}

	size_t numEdges = faceIn->nEdges();
	if( numEdges > geomSettings->m_maxNumFaceEdges )
	{
		return;
	}

	for( size_t ii = 0; ii < numEdges; ++ii )
	{
		if( edge->rev )
		{
			carve::mesh::Face<3>* adjacentFace = edge->rev->face;

			bool alreadyHandled = false;
			for( auto existingFacePair : setAdjacentFaces )
			{
				if( existingFacePair->faceOnEdge == adjacentFace || existingFacePair->faceOnReverseEdge == adjacentFace )
				{
					alreadyHandled = true;
					break;
				}
			}

			if( faceIn == adjacentFace)
			{
				alreadyHandled = true;
			}

			if( !alreadyHandled )
			{
				if( isCoplanar(plane, faceIn, adjacentFace, geomSettings) )
				{
					shared_ptr<AdjacentFacePair> facePair(new AdjacentFacePair(faceIn, adjacentFace, edge));
					setAdjacentFaces.insert(facePair);

					if( recursive )
					{
						getAdjacentCoplanarFaces(plane, adjacentFace, setAdjacentFaces, geomSettings, recursive);
					}
				}
			}
		}

		edge = edge->next;
		if( edge == faceIn->edge )
		{
			break;
		}
	}

}

static void _link(carve::mesh::Edge<3>* a, carve::mesh::Edge<3>* b)
{
	a->next = b;
	b->prev = a;
}
static void _setloopface(carve::mesh::Edge<3>* s, carve::mesh::Face<3>* f)
{
	carve::mesh::Edge<3>* e = s;
	do
	{
		e->face = f;
		e = e->next;
	} while (e != s);
}

static carve::mesh::Edge<3>* checkMergeFaces(carve::mesh::Edge<3>* e, shared_ptr<GeometrySettings>& geomSettings, bool dumpMeshes)
{
	if (e->rev == nullptr)
	{
		return nullptr;
	}


	carve::mesh::Face<3>* fwdface = e->face;
	carve::mesh::Face<3>* revface = e->rev->face;

	if( fwdface == revface )
	{
		return nullptr;
	}

	if( fwdface->n_edges > geomSettings->m_maxNumFaceEdges )
	{
		return nullptr;
	}
	if( revface->n_edges > geomSettings->m_maxNumFaceEdges )
	{
		return nullptr;
	}

	size_t n_removed = 0;

	carve::mesh::Edge<3>* splice_beg = e;
	for( size_t ii = 0; ii < fwdface->n_edges; ++ii )
	{
		splice_beg = splice_beg->prev;
		++n_removed;

		if( splice_beg == e) { break; }
		if( !splice_beg->rev ) { break; }
		if( splice_beg->next->rev->prev != splice_beg->rev ) { break; }
	}

	if (splice_beg == e)
	{
		// edge loops are completely matched.
		return nullptr;
	}

	carve::mesh::Edge<3>* splice_end = e;
	do {
		splice_end = splice_end->next;
		++n_removed;
	} while (splice_end->rev && splice_end->prev->rev->next == splice_end->rev);

	--n_removed;

	carve::mesh::Edge<3>* link1_p = splice_beg;
	carve::mesh::Edge<3>* link1_n = splice_beg->next->rev->next;

	carve::mesh::Edge<3>* link2_p = splice_end->prev->rev->prev;
	carve::mesh::Edge<3>* link2_n = splice_end;

	CARVE_ASSERT(link1_p->face == fwdface);
	CARVE_ASSERT(link1_n->face == revface);

	CARVE_ASSERT(link2_p->face == revface);
	CARVE_ASSERT(link2_n->face == fwdface);

	carve::mesh::Edge<3>* left_loop = link1_p->next;

	CARVE_ASSERT(left_loop->rev == link1_n->prev);

	_link(link2_n->prev, link1_p->next);
	_link(link1_n->prev, link2_p->next);

	_link(link1_p, link1_n);
	_link(link2_p, link2_n);

	fwdface->edge = link1_p;

	size_t edgeCount = 0;
	for (carve::mesh::Edge<3>* e = link1_n; e != link2_n; e = e->next)
	{
		CARVE_ASSERT(e->face == revface);
		e->face = fwdface;
		fwdface->n_edges++;
		++edgeCount;
		if( edgeCount > geomSettings->m_maxNumFaceEdges )
		{
			std::logic_error ex("edgeCount > m_maxNumFaceEdges");
			throw std::exception(ex);
			return nullptr;
		}
	}
	edgeCount = 0;
	for (carve::mesh::Edge<3>* e = link2_n; e != link1_n; e = e->next)
	{
		CARVE_ASSERT(e->face == fwdface);
		++edgeCount;
		if( edgeCount > geomSettings->m_maxNumFaceEdges )
		{
			std::logic_error ex("edgeCount > m_maxNumFaceEdges");
			throw std::exception(ex);
			return nullptr;
		}
	}

	fwdface->n_edges -= n_removed;

	revface->n_edges = 0;
	revface->edge = nullptr;

	_setloopface(left_loop, nullptr);
	_setloopface(left_loop->rev, nullptr);

	return left_loop;
}

static size_t findAndMergeCoplanarFaces( carve::mesh::Face<3>* faceIn, std::set<carve::mesh::Face<3>*>& setAllFaces, shared_ptr<CoplanarFaceContainer> coplanar, shared_ptr<GeometrySettings>& geomSettings, bool dumpFaces, double CARVE_EPSILON)
{
	double faceArea = MeshUtils::computeFaceArea(faceIn);
	if( std::abs(faceArea) < CARVE_EPSILON*10 )
	{
		return 0;
	}

	carve::mesh::Edge<3>* edge = faceIn->edge;
	vec3& facePosition_carve = edge->v2()->v;

	const carve::geom::plane<3>& plane = faceIn->plane;
	const vec3& faceNormal_carve = faceIn->plane.N;
	glm::dvec3 faceNormal(faceNormal_carve.x, faceNormal_carve.y, faceNormal_carve.z);
	glm::dvec3 facePosition(facePosition_carve.x, facePosition_carve.y, facePosition_carve.z);

	std::set<shared_ptr<AdjacentFacePair> > setAdjacentCoplanarFaces;
	bool recursive = true;

	getAdjacentCoplanarFaces(plane, faceIn, setAdjacentCoplanarFaces, geomSettings, recursive);

	size_t numChanges = 0;
	size_t numFacesDeleted = 0;
	size_t numFacesInSetAllFaces = setAllFaces.size();

	std::set<carve::mesh::Mesh<3>* > setMeshesToFix;
	coplanar->m_face = faceIn;
	for( auto it1 = setAdjacentCoplanarFaces.begin(); it1 != setAdjacentCoplanarFaces.end(); ++it1 )
	{
		shared_ptr<AdjacentFacePair> facePair = *it1;
		carve::mesh::Face<3>* faceOnRverseEdge = facePair->faceOnReverseEdge;
		carve::mesh::Face<3>* faceOnEdge = facePair->faceOnEdge;
		if( faceOnEdge == nullptr )
		{
			continue;
		}

		if( faceOnRverseEdge == nullptr )
		{
			continue;
		}

		if( facePair->edge == nullptr )
		{
			continue;
		}

		if( faceOnEdge == faceOnRverseEdge )
		{
			// remaining edge between inner opening of face and outer boundary. Leave it for triangulation
			//continue;
		}
		size_t numFacesBeforeMerge = faceOnRverseEdge->mesh->faces.size();

		carve::mesh::Edge<3>* edgeErase = facePair->edge;
		size_t numEdgesFace = countEdges(faceOnEdge);
		size_t numEdgesFaceReverse = countEdges(faceOnRverseEdge);

		if( numEdgesFace >= geomSettings->m_maxNumFaceEdges )
		{
			std::logic_error ex("edgeCount > m_maxNumFaceEdges");
			throw std::exception(ex);
		}
		if( numEdgesFaceReverse >= geomSettings->m_maxNumFaceEdges )
		{
			std::logic_error ex("edgeCount > m_maxNumFaceEdges");
			throw std::exception(ex);
		}

		MeshSetInfo info2;
		bool checkForDegenerateEdges = true;
		bool valid2 = MeshUtils::checkFaceIntegrity(edgeErase->face, checkForDegenerateEdges, info2);

		MeshSetInfo info3;
		bool valid3 = MeshUtils::checkFaceIntegrity(edgeErase->rev->face, checkForDegenerateEdges, info3);

		if( !valid2 )
		{
			continue;
		}
		if( !valid3 )
		{
			continue;
		}

		std::set<carve::mesh::Face<3>* > adjacentFaces;
		getAdjacentFaces(faceOnEdge, adjacentFaces);

		carve::mesh::Edge<3>* edgeMergeNext = MeshOps::checkMergeFaces(edgeErase, geomSettings, dumpFaces);

		if( !edgeMergeNext )
		{
			continue;
		}

		carve::mesh::Face<3>* faceRemain = nullptr;
		carve::mesh::Face<3>* faceRemove = nullptr;

		if( faceOnRverseEdge->edge )
		{
			faceRemain = faceOnRverseEdge;
			size_t numEdgesFaceRemove_afterMerge = countEdges(faceOnRverseEdge);
			if( numEdgesFaceRemove_afterMerge > geomSettings->m_maxNumFaceEdges )
			{
				std::logic_error ex("faceOnRverseEdge->edge count > maxNumFaceEdges");
				throw std::exception(ex);
				continue;
			}
			if( numEdgesFaceReverse != numEdgesFaceRemove_afterMerge )
			{
				++numChanges;
			}
		}
		else
		{
			faceRemove = faceOnRverseEdge;
		}

		if( faceOnEdge->edge )
		{
			faceRemain = faceOnEdge;
			size_t numEdgesFaceRemain_afterMerge = countEdges(faceOnEdge);
			if( numEdgesFaceRemain_afterMerge > geomSettings->m_maxNumFaceEdges )
			{
				std::logic_error ex("faceOnEdge->edge count > maxNumFaceEdges");
				throw std::exception(ex);
				continue;
			}
			if( numEdgesFace != numEdgesFaceRemain_afterMerge )
			{
				++numChanges;
			}
		}
		else
		{
			faceRemove = faceOnEdge;
		}

		// replace pointers to faceRemove
		for( auto it2 = setAdjacentCoplanarFaces.begin(); it2 != setAdjacentCoplanarFaces.end(); ++it2 )
		{
			shared_ptr<AdjacentFacePair> facePair2 = *it2;
			if( facePair == facePair2 )
			{
				continue;
			}
			if( facePair2->faceOnEdge == faceRemove )
			{
				facePair2->faceOnEdge = faceRemain;
			}
			if( facePair2->faceOnReverseEdge == faceRemove )
			{
				facePair2->faceOnReverseEdge = faceRemain;
			}
		}

		auto itFindFace = setAllFaces.find(faceRemove);
		if( itFindFace != setAllFaces.end() )
		{
			setAllFaces.erase(itFindFace);
		}
		else
		{
		}

		size_t numPointersToFaceDelete = replaceFacePointer(faceRemove, faceRemain, adjacentFaces);
		if( numPointersToFaceDelete > 0)
		{
			std::cout << "numPointersToFaceDelete > 0" << std::endl;
		}
		numChanges += removeFaceFromMesh(faceRemove);
		delete faceRemove;
		++numFacesDeleted;

		facePair->faceOnReverseEdge = nullptr;
		facePair->faceOnEdge = nullptr;
		facePair->edge = nullptr;
		facePair->edgeDeleted = true;

		if( !faceRemain )
		{
			continue;
		}

		auto mesh = faceRemain->mesh;
		try
		{
			faceRemain->edge->validateLoop();
		}
		catch( carve::exception& e )
		{
			std::cout << "validateLoop failed: " << e.str();
		}

		IFCC::MeshUtils::checkFaceLoops(mesh);

		mesh->cacheEdges();
		mesh->recalc(CARVE_EPSILON);
		setMeshesToFix.insert(mesh);

		++numChanges;

		// TODO: enforceMergedFacesToCommonPlane() : compute normal vector and centroid of merged face, then move all vertices precisely into that plane

		std::vector<carve::mesh::Vertex<3>* > faceVertices;
		faceRemain->getVertices(faceVertices);
		if( faceVertices.size() > 3 )
		{
			carve::geom::vector<3> normalVector = GeomUtils::computePolygonNormal(faceVertices);
			carve::geom::vector<3> centroid = faceRemain->centroid();
			GeomUtils::Plane plane( glm::dvec3(centroid.x, centroid.y, centroid.z), glm::dvec3(normalVector.x, normalVector.y, normalVector.z) );
			bool allVerticesInPlane = true;
			for( carve::mesh::Face<3>::vertex_t * vertex : faceVertices )
			{
				const carve::geom::vector<3>& facePoint_carve = vertex->v;
				glm::dvec3 facePoint(facePoint_carve.x, facePoint_carve.y, facePoint_carve.z);

				double distanceToPlane = plane.distancePointPlane(facePoint);

				if( std::abs(distanceToPlane) > geomSettings->getEpsilonCoplanarDistance() )// m_epsCoplanarDistance )
				{
					vec3 pointOnPlane = facePoint_carve + normalVector * distanceToPlane;
					vertex->v = pointOnPlane;

				}
			}

			if( allVerticesInPlane )
			{
				return true;
			}
		}

	}

	size_t numFacesInSetAllFaces1 = setAllFaces.size();
	if( numFacesDeleted != numFacesInSetAllFaces - numFacesInSetAllFaces1)
	{
		std::cout << "numFacesDeleted != numFacesInSetAllFaces - numFacesInSetAllFaces1" << std::endl;
	}

	return numChanges;
}

size_t mergeCoplanarFacesInMeshSet( shared_ptr<carve::mesh::MeshSet<3> >& meshset, shared_ptr<GeometrySettings>& geomSettings, bool shouldBeClosedManifold, bool dumpFaces, double CARVE_EPSILON)
{
	shared_ptr<carve::mesh::MeshSet<3> > meshset_copy(meshset->clone());

	std::set<carve::mesh::Face<3>* > setFacesBegin;
	MeshUtils::getFacesInMeshSet(meshset, setFacesBegin);
	size_t numFacesAll = setFacesBegin.size();
	size_t numChanges = 0;

	size_t maxNumFacesToMerge = 600;
	if( numFacesAll > maxNumFacesToMerge )
	{
		numFacesAll = maxNumFacesToMerge;
	}

	std::set<carve::mesh::Face<3>* > setMasterFaces;  // remaining faces
	for( size_t ii = 0; ii < numFacesAll; ++ii )
	{
		std::set<carve::mesh::Face<3>* > setFaces;
		MeshUtils::getFacesInMeshSet(meshset, setFaces);
		if( setFaces.size() == setMasterFaces.size() )
		{
			break;
		}

		size_t numChangesCurrentLoop = 0;
		size_t jj = 0;
		for( auto it = setFaces.begin(); it != setFaces.end(); ++it )
		{
			carve::mesh::Face<3>* face = *it;
			if( !face )
			{
				continue;
			}

			if( jj > maxNumFacesToMerge )
			{
				break;
			}
			++jj;

			auto itFindInMaster = setMasterFaces.find(face);
			if( itFindInMaster != setMasterFaces.end() )
			{
				continue;
			}

			carve::mesh::Edge<3>* edge = face->edge;
			if( !edge )
			{
				continue;
			}

			double faceArea = MeshUtils::computeFaceArea(face);
			if( std::abs(faceArea) < CARVE_EPSILON*10 )
			{
				continue;
			}

			vec3& facePosition_carve = edge->v2()->v;

			const vec3& faceNormal_carve = face->plane.N;
			glm::dvec3 faceNormal(faceNormal_carve.x, faceNormal_carve.y, faceNormal_carve.z);
			glm::dvec3 facePosition(facePosition_carve.x, facePosition_carve.y, facePosition_carve.z);
			shared_ptr<CoplanarFaceContainer> coplanar(new CoplanarFaceContainer());
			coplanar->m_pointProjector.m_plane.setNormal(faceNormal);
			coplanar->m_pointProjector.m_plane.setPlane(facePosition, faceNormal);

			size_t numFaces = setFaces.size();
			size_t numChangesMergedFaces = findAndMergeCoplanarFaces(face, setFaces, coplanar, geomSettings, dumpFaces, CARVE_EPSILON);
			numChanges += numChangesMergedFaces;
			numChangesCurrentLoop += numChangesMergedFaces;

			setMasterFaces.insert(face);
		}
	}

	MeshUtils::fixMeshset(meshset.get(), CARVE_EPSILON, shouldBeClosedManifold, dumpFaces);

	for( auto mesh : meshset->meshes )
	{
		mesh->cacheEdges();
		mesh->recalc(CARVE_EPSILON);
	}

	MeshSetInfo infoResult;
	bool validMeshsetResult = MeshUtils::checkMeshSetValidAndClosed(meshset, infoResult, CARVE_EPSILON);

	if( !validMeshsetResult )
	{
		meshset = meshset_copy;
		numChanges = 0;
	}

	return numChanges;
}

static void getPointerToEdge(carve::mesh::Mesh<3>* mesh, carve::mesh::Edge<3>* edgeFind, std::set<carve::mesh::Edge<3>* >& setEdges, std::set<carve::mesh::Face<3>* >& setFaces)
{
	size_t numEdges = 0;
	for( carve::mesh::Face<3>* face : mesh->faces )
	{
		if( !face )
		{
			continue;
		}

		carve::mesh::Edge<3>* edge = face->edge;
		if( edge == edgeFind )
		{
			setFaces.insert(face);
		}

		size_t numEdgesCurrentFace = face->n_edges;
		for( size_t ii = 0; ii < numEdgesCurrentFace; ++ii )
		{
			if( !edge )
			{
				continue;
			}
			if( edge->next == edgeFind )
			{
				setEdges.insert(edge);
			}
			if( edge->prev == edgeFind )
			{
				setEdges.insert(edge);
			}
			if( edge->rev == edgeFind )
			{
				setEdges.insert(edge);
			}
			edge = edge->next;
		}
	}
}

static void getEdgesOnVertex(carve::mesh::Mesh<3>* mesh, carve::mesh::Vertex<3>* vertex, std::set<carve::mesh::Edge<3>* >& setEdges)
{
	size_t numEdges = 0;
	for( carve::mesh::Face<3>* face : mesh->faces )
	{
		if( !face )
		{
			continue;
		}

		carve::mesh::Edge<3>* edge = face->edge;
		size_t numEdgesCurrentFace = face->n_edges;
		for( size_t ii = 0; ii < numEdgesCurrentFace; ++ii )
		{
			if( !edge )
			{
				continue;
			}
			if( edge->vert == vertex )
			{
				setEdges.insert(edge);
			}
			edge = edge->next;
		}
	}
}

static size_t removePointerToVertex(carve::mesh::Mesh<3>* mesh, carve::mesh::Vertex<3>* vertRemove, carve::mesh::Vertex<3>* vertReplace)
{
	size_t numChanges = 0;
	for( carve::mesh::Face<3>* face : mesh->faces )
	{
		if( !face )
		{
			continue;
		}

		carve::mesh::Edge<3>* edge = face->edge;
		size_t numEdgesCurrentFace = face->n_edges;
		for( size_t ii = 0; ii < numEdgesCurrentFace; ++ii )
		{
			if( !edge )
			{
				continue;
			}
			if( edge->vert == vertRemove )
			{
				edge->vert = vertReplace;
				++numChanges;
			}
			edge = edge->next;
		}
	}
	return numChanges;
}

static size_t mergeAlignedEdges(shared_ptr<carve::mesh::MeshSet<3> >& meshset, double epsAlignmentAngle, bool dumpFaces, double CARVE_EPSILON)
{

	size_t numEdgesRemoved = 0;
	for( carve::mesh::Mesh<3>* mesh : meshset->meshes )
	{
		for( carve::mesh::Face<3>*face : mesh->faces )
		{
			if( !face )
			{
				continue;
			}

			if( !face->edge )
			{
				continue;
			}

			bool faceDumped = false;
			carve::mesh::Edge<3>* edge = face->edge;

			// check how many edges are connected to end of edge
			size_t numEdges = face->n_edges;
			for( size_t ii = 0; ii < numEdges; ++ii )
			{
				if( !edge )
				{
					continue;
				}
				if( edge->next )
				{
					if( edge->next->rev )
					{
						if( edge->next->rev->next )
						{
							if( edge->next->rev->next->rev == edge )
							{
								// only one edge is connected, now check angle

								//      edge->rev->next         edge->rev            edge->next->rev        edge->next->next->rev
								//  <--------------------p1<------------------p2<-------------------------p3<-----------------------
								//   ------------------->   ------------------>   ----------------------->   ---------------------->
								//     edge->prev               edge                  edge->next             edge->next->next

								carve::mesh::Vertex<3>* vertex1 = edge->v1();
								carve::mesh::Vertex<3>* vertex2 = edge->v2();
								carve::mesh::Vertex<3>* vertex3 = edge->next->v2();

								std::set<carve::mesh::Edge<3>* > setEdges;
								getEdgesOnVertex(mesh, vertex2, setEdges);
								size_t numEdgesOnVertex = setEdges.size();

								const carve::geom::vector<3>& p1 = vertex1->v;
								const carve::geom::vector<3>& p2 = vertex2->v;
								const carve::geom::vector<3>& p3 = vertex3->v;

								carve::geom::vector<3> edgeVector = p2 - p1;
								carve::geom::vector<3> edgeNextVector = p3 - p2;
								edgeVector.normalize();
								edgeNextVector.normalize();


								double dotProduct = dot(edgeVector, edgeNextVector);
								if( std::abs(dotProduct - 1.0) < epsAlignmentAngle*1000 )
								{
									carve::mesh::Edge<3>* edgeRemove = edge;// ->next;


									std::set<carve::mesh::Edge<3>* > setEdgePointersToRemovedEdge;
									std::set<carve::mesh::Face<3>* > setFacePointersToRemovedEdge;
									getPointerToEdge(mesh, edgeRemove, setEdgePointersToRemovedEdge, setFacePointersToRemovedEdge);
									size_t numVertexChanges = removePointerToVertex(mesh, vertex2, vertex1);
									edge = edgeRemove->removeEdge();  // returns ->next
									carve::geom::vector<3> distanceV1 = edge->v1()->v - p1;
									carve::geom::vector<3> distanceV3 = edge->v2()->v - p3;

									double epsMinFaceArea = CARVE_EPSILON * 0.001;
									MeshUtils::removeZeroAreaFacesInMesh(mesh, epsMinFaceArea, CARVE_EPSILON);

									++numEdgesRemoved;
									mesh->cacheEdges();
									mesh->recalc(CARVE_EPSILON);

									//      edge->rev->next         edge->rev                                    edge->next->rev
									//  <--------------------v1<---------------------------------------------v2<------------------------
									//   ------------------->   -------------------------------------------->   ---------------------->
									//     edge->prev               edge                                           edge->next


									continue;
								}
							}
						}
					}
					edge = edge->next;
				}
			}

		}
	}

	if( numEdgesRemoved > 0 )
	{

		for( auto mesh : meshset->meshes )
		{
			mesh->cacheEdges();
			mesh->recalc(CARVE_EPSILON);

		}
	}

	return numEdgesRemoved;
}

/// \brief simplifyMeshSet merge coplanar faces and re-triangulate each set of merged faces
/// \param meshset				Carve meshset
/// \param ignoreOpenEdgesInResult	If true, the result is kept even with open edges (good for visualization). If false, the result will be the input mesh in case open edges occur after triangulation (good for further boolean operations)
void simplifyMeshSet( shared_ptr<carve::mesh::MeshSet<3> >& meshset, shared_ptr<GeometrySettings>& geomSettings,
	bool triangulateResult, bool shouldBeClosedManifold, bool dumpPolygon, double CARVE_EPSILON)
{
	if( !meshset )
	{
		return;
	}
	double epsCoplanarDistance = geomSettings->getEpsilonCoplanarDistance();// m_epsCoplanarDistance;
	double epsCoplanarAngle = geomSettings->getEpsilonCoplanarAngle();// m_epsCoplanarAngle;
	double epsMinFaceArea = geomSettings->getEpsilonCoplanarDistance() * 0.01;// m_epsCoplanarDistance * 0.001;
	MeshSetInfo infoInput;
	bool validMeshsetInput = MeshUtils::checkMeshSetValidAndClosed(meshset, infoInput, CARVE_EPSILON);

	if( meshset->vertex_storage.size() < 9 && infoInput.numOpenEdges == 0 )
	{
		return;
	}

	if( !validMeshsetInput )
	{
		MeshUtils::resolveOpenEdges(meshset, epsCoplanarDistance, dumpPolygon);
		validMeshsetInput = MeshUtils::checkMeshSetValidAndClosed(meshset, infoInput, CARVE_EPSILON);

	}


	bool meshShouldBeTriangulated = false;
	if( meshShouldBeTriangulated )
	{
		bool meshTriangulated = checkMeshsetTriangulated(meshset);
		if( !meshTriangulated )
		{
			shared_ptr<carve::mesh::MeshSet<3> > meshset_copy(meshset->clone());
			retriangulateMeshSetSimple(meshset, false, epsCoplanarDistance, 0);
			meshTriangulated = checkMeshsetTriangulated(meshset);
			if( !meshTriangulated )
			{
				//return;
			}

			bool validMeshsetTriangulated = MeshUtils::checkMeshSetValidAndClosed(meshset, infoInput, CARVE_EPSILON);
			if( !validMeshsetTriangulated )
			{
				meshset = meshset_copy;
			}
		}
	}

	shared_ptr<carve::mesh::MeshSet<3> > meshset_copy(meshset->clone());
	dumpPolygon = false;

	try
	{
		MeshUtils::removeZeroAreaFacesInMeshSet(meshset, epsMinFaceArea, CARVE_EPSILON);
		size_t numChanges = mergeCoplanarFacesInMeshSet(meshset, geomSettings, shouldBeClosedManifold, dumpPolygon, CARVE_EPSILON);
		MeshUtils::recalcMeshSet(meshset, CARVE_EPSILON);

		// TODO: find faces with biggest area, and trim all points to plane

		MeshSetInfo infoMergedFaces;
		bool validMeshsetMergedFaces = MeshUtils::checkMeshSetValidAndClosed(meshset, infoMergedFaces, CARVE_EPSILON);


		if( !validMeshsetMergedFaces )
		{
			meshset = meshset_copy;
			return;
		}

		MeshUtils::recalcMeshSet(meshset, CARVE_EPSILON);

		shared_ptr<carve::mesh::MeshSet<3> > meshset_next = shared_ptr<carve::mesh::MeshSet<3> >(meshset->clone());

		// run the check again with the new
		validMeshsetMergedFaces = MeshUtils::checkMeshSetValidAndClosed(meshset_next, infoMergedFaces, CARVE_EPSILON);

		if( !validMeshsetMergedFaces )
		{
			meshset = meshset_copy;
			return;
		}

		meshset_copy = meshset_next;

		size_t numEdgesRemoved = mergeAlignedEdges(meshset, epsCoplanarAngle, false, CARVE_EPSILON);
		if( numEdgesRemoved > 0 )
		{
			MeshSetInfo infoMergedAlignedEdges;
			bool validMergedAlignedEdges = MeshUtils::checkMeshSetValidAndClosed(meshset, infoMergedAlignedEdges, CARVE_EPSILON);

			if( validMergedAlignedEdges )
			{
				if( triangulateResult )
				{
					retriangulateMeshSetSimple(meshset, false, epsCoplanarDistance, 0);
					MeshSetInfo infoTriangulated;
					bool validTriangulated = MeshUtils::checkMeshSetValidAndClosed(meshset, infoTriangulated, CARVE_EPSILON);
					if( !validTriangulated )
					{
						meshset = meshset_copy;
						return;
					}
				}
				return;
			}
		}

		return;
	}
	catch(std::exception& ex)
	{
	}
	catch( carve::exception& ex )
	{
		std::cout << ex.str() << std::endl;
	}
	catch( ... )
	{

	}

	meshset = meshset_copy;
}

void simplifyMeshSet(std::vector<shared_ptr<carve::mesh::MeshSet<3>> >& meshsets, shared_ptr<GeometrySettings>& geomSettings,
	bool triangulateResult, bool shouldBeClosedManifold, bool dumpPolygon, double CARVE_EPSILON)
{
	for( shared_ptr<carve::mesh::MeshSet<3> >&meshset : meshsets )
	{
		simplifyMeshSet(meshset, geomSettings, triangulateResult, shouldBeClosedManifold, dumpPolygon, CARVE_EPSILON);
	}
}

} // end namespace Mesh_Ops

} // end namespace IFCC
