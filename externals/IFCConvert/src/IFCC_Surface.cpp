#include "IFCC_Surface.h"

#include <limits>
#include <fstream>
#include <sstream>

#include <IBK_math.h>
#include <IBK_assert.h>

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
#include "IFCC_Clippertools.h"
#include "IFCC_Helper.h"

//#define SURFACE_DUMP

namespace IFCC {

Surface::Surface() :
	m_id(-1),
	m_elementEntityId(-1),
	m_positionType(PT_Unknown),
	m_virtualSurface(false)
{
}

Surface::Surface(carve::mesh::Face<3>* face) :
	m_id(-1),
	m_elementEntityId(-1),
	m_positionType(PT_Unknown),
	m_virtualSurface(false)
{
	IBK_ASSERT(face != nullptr);

	std::vector<carve::mesh::Vertex<3>* > vertices;
	face->getVertices(vertices);

	for(size_t vi=0; vi<vertices.size(); ++vi) {
		double x = vertices[vi]->v.x;
		double y = vertices[vi]->v.y;
		double z = vertices[vi]->v.z;
		m_polyVect.emplace_back(IBKMK::Vector3D(x,y,z));
	}
	m_planeCarve = face->plane;
	PlaneHesseNormal hesse(face->plane);
	m_planeNormal = PlaneNormal(hesse, m_polyVect);

}

Surface::Surface(const polygon3D_t& polygon) :
	m_id(-1),
	m_elementEntityId(-1),
	m_positionType(PT_Unknown),
	m_virtualSurface(false),
	m_polyVect(polygon)
{
	if(m_polyVect.size() > 2) {
		m_planeNormal = PlaneNormal(polygon);
		PlaneHesseNormal planeHesseNormal(polygon);
		m_planeCarve.N = carve::geom::VECTOR(planeHesseNormal.m_n0.m_x,planeHesseNormal.m_n0.m_y,planeHesseNormal.m_n0.m_z);
		m_planeCarve.d = planeHesseNormal.m_d;
	}
}

void Surface::set(int id, int elementId, const std::string& name, bool isVirtual) {
	m_id = id;
	m_elementEntityId = elementId;
	m_name = name;
	m_virtualSurface = isVirtual;
}

double Surface::distanceToParallelPlane(const Surface& other) const {
//	double negFact = 1.0;
//	if(!nearEqual(m_planeNormal.m_lz, other.m_planeNormal.m_lz)) {
//		if(!nearEqual(m_planeNormal.m_lz*-1,other.m_planeNormal.m_lz))
//			return std::numeric_limits<double>::max();
//		else
//			negFact = -1.0;
//	}

//	double dist = std::fabs(other.m_planeNormal.m_distance - (m_planeNormal.m_distance * negFact));

//	IBKMK::Vector3D t = m_polyVect[0] - other.m_polyVect[0];

	PlaneHesseNormal phn1(m_polyVect);
	PlaneHesseNormal phn2(other.m_polyVect);
	double dist2 = std::fabs(phn1.m_d - phn2.m_d);
//	if(dist2<dist) {
//		int check = 0;
//	}

//	double dist2 = t.scalarProduct(PlaneHesseNormal(m_polyVect).m_n0);

	return dist2;
}

bool Surface::isParallelTo(const Surface& other) const {
	double a;
	double b;
	double c;

	carve::geom::vector<3> v1 = m_planeCarve.N;
	carve::geom::vector<3> v2 = other.m_planeCarve.N;

	if(IBK::near_zero(v2.x)) {
		if(!IBK::near_zero(v1.x))
			return false;
		a = 1;
	}
	else {
		a = v1.x / v2.x;
	}
	if(IBK::near_zero(v2.y)) {
		if(!IBK::near_zero(v1.y))
			return false;
		b = 1;
	}
	else {
		b = v1.y / v2.y;
	}
	if(IBK::near_zero(v2.z)) {
		if(!IBK::near_zero(v1.z))
			return false;
		c = 1;
	}
	else {
		c = v1.z / v2.z;
	}
	if(IBK::near_equal(a,b) && IBK::near_equal(a,c)) {
		return true;
	}

	carve::geom::vector<3> negV1 = v1.negated();
	if(IBK::near_zero(v2.x)) {
		if(!IBK::near_zero(negV1.x))
			return false;
		a = 1;
	}
	else {
		a = negV1.x / v2.x;
	}
	if(IBK::near_zero(v2.y)) {
		if(!IBK::near_zero(negV1.y))
			return false;
		b = 1;
	}
	else {
		b = negV1.y / v2.y;
	}
	if(IBK::near_zero(v2.z)) {
		if(!IBK::near_zero(negV1.z))
			return false;
		c = 1;
	}
	else {
		c = negV1.z / v2.z;
	}
	if(IBK::near_equal(a,b) && IBK::near_equal(a,c)) {
		return true;
	}
	return false;
}

bool Surface::isEqualTo(const Surface& other) const {
	const std::vector<IBKMK::Vector3D>& otherPoly = other.polygon();
	if(m_polyVect.size() != otherPoly.size())
		return false;

	// search point in the other polygon which is equal to the first point of the current one
	// store the position in start2nd
	int start2nd = -1;
	for(size_t i=0; i<otherPoly.size(); ++i) {
		if(nearEqual(otherPoly[i],m_polyVect.front()))
			start2nd = i;
	}
	// no equal point found - polygones are not equal
	if(start2nd == -1)
		return false;

	// first point of both polygons is the same - perform normal search for all other points
	if(start2nd == 0) {
		for(size_t i=1; i<m_polyVect.size(); ++i) {
			if(!nearEqual(m_polyVect[i], otherPoly[i]))
				return false;
		}
	}
	// first point of the current polygon is equal to a later point of the other one
	else {
		// copy the other point and rotate it so the first point is the same as in the current one
		std::vector<IBKMK::Vector3D> otherCopy = other.polygon();
		std::rotate(otherCopy.begin(), otherCopy.begin() + start2nd, otherCopy.end());
		// check if all other points are equal
		for(size_t i=1; i<m_polyVect.size(); ++i) {
			if(!nearEqual(m_polyVect[i], otherCopy[i]))
				return false;
		}
	}
	return true;
}


std::vector<std::pair<size_t,size_t>> Surface::samePoints(const Surface& other) const {
	std::vector<std::pair<size_t,size_t>> equalPoints;
	const std::vector<IBKMK::Vector3D>& otherPoly = other.polygon();
	for(size_t i=0; i<m_polyVect.size(); ++i) {
		for(size_t j=0; j<otherPoly.size(); ++j) {
			if(nearEqual(m_polyVect[i], otherPoly[j]))
				equalPoints.push_back(std::make_pair(i,j));
		}
	}
	return equalPoints;
}

bool Surface::isIntersected(const Surface& other) const {
	polygon3D_t result = intersectPolygons(m_polyVect, other.polygon(), m_planeNormal);
	if(result.empty() || areaPolygon(result) < 1e-4)
		return false;
	return true;
//	return intersects(m_polyVectOrg, other.polygon());
}

Surface Surface::intersect(const Surface& other) const {
	polygon3D_t result = intersectPolygons(m_polyVect, other.polygon(), m_planeNormal);
	if(result.empty() || areaPolygon(result) < 1e-4)
		return Surface();

	return Surface(result);
}

Surface::IntersectionResult Surface::intersect2(const Surface& other) const {
	IFCC::IntersectionResult tmp = intersectPolygons2(m_polyVect, other.polygon(), m_planeNormal);
	Surface::IntersectionResult result;
	for(const polygon3D_t& poly : tmp.m_intersections) {
		if(poly.size() > 3 && areaPolygon(poly) > 1e-4)
			result.m_intersections.push_back(Surface(poly));
	}
	for(size_t i=0; i<tmp.m_diffBaseMinusClip.size(); ++i) {
		const polygon3D_t& poly = tmp.m_diffBaseMinusClip[i];
		if(poly.size() > 3 && areaPolygon(poly) > 1e-4) {
			result.m_diffBaseMinusClip.push_back(Surface(poly));
			result.m_holesBaseMinusClip.push_back(std::vector<Surface>());
			for(const polygon3D_t& hole : tmp.m_holesBaseMinusClip[i]) {
				if(hole.size() > 3 && areaPolygon(hole) > 1e-4)
					result.m_holesBaseMinusClip.back().push_back(hole);
			}
		}
	}
	for(size_t i=0; i<tmp.m_diffClipMinusBase.size(); ++i) {
		const polygon3D_t& poly = tmp.m_diffClipMinusBase[i];
		if(poly.size() > 3 && areaPolygon(poly) > 1e-4) {
			result.m_diffClipMinusBase.push_back(Surface(poly));
			result.m_holesClipMinusBase.push_back(std::vector<Surface>());
			for(const polygon3D_t& hole : tmp.m_holesClipMinusBase[i]) {
				if(hole.size() > 3 && areaPolygon(hole) > 1e-4)
					result.m_holesClipMinusBase.back().push_back(hole);
			}
		}
	}
	return result;
}

std::vector<Surface> Surface::difference(const Surface& other) const {
	IFCC::IntersectionResult tmp = intersectPolygons2(m_polyVect, other.polygon(), m_planeNormal);
	std::vector<Surface> result;
	for(size_t i=0; i<tmp.m_diffBaseMinusClip.size(); ++i) {
		const polygon3D_t& poly = tmp.m_diffBaseMinusClip[i];
		if(poly.size() > 3 && areaPolygon(poly) > 1e-4) {
			result.push_back(Surface(poly));
		}
	}
	return result;
}


bool Surface::merge(const Surface& subsurface) {
	polygon3D_t result = mergePolygons(m_polyVect, subsurface.polygon(), m_planeNormal);
	if(result.empty())
		return false;

	m_polyVect = result;
	return true;
}

bool Surface::addSubSurface(const Surface& subsurface) {
	SubSurface sub(subsurface.polygon(), *this);
	if(!sub.isValid())
		return false;

	sub.set(subsurface.id(), subsurface.m_name, subsurface.m_elementEntityId);
	m_subSurfaces.push_back(sub);
	return true;
}

double Surface::area() const {
	return areaPolygon(m_polyVect);
}

double Surface::signedArea() const {
	polygon2D_t poly2D = polygon2DInPlane();
	return areaSignedPolygon(poly2D);
}

polygon2D_t Surface::polygon2DInPlane() const {
	polygon2D_t polygon2DBase;
	for(const IBKMK::Vector3D& vect : m_polyVect) {
		IBKMK::Vector2D p2 = m_planeNormal.convert3DPoint(vect);
		polygon2DBase.push_back(p2);
	}

	return polygon2DBase;
}

void Surface::flip(bool positive) {
	std::reverse(m_polyVect.begin(), m_polyVect.end());
	for(auto& sub : m_subSurfaces) {
		sub.flip(positive);
	}
}

TiXmlElement * Surface::writeXML(TiXmlElement * parent) const {
	if (m_id == -1)
		return nullptr;

	TiXmlElement * e = new TiXmlElement("Surface");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
//	e->SetAttribute("visible", IBK::val2string<bool>(true));

	if(!m_polyVect.empty()) {
		TiXmlElement * child = new TiXmlElement("Polygon3D");
		e->LinkEndChild(child);

		std::stringstream vals;
		for (unsigned int i=0; i<m_polyVect.size(); ++i) {
			vals << m_polyVect[i].m_x << " " << m_polyVect[i].m_y << " " << m_polyVect[i].m_z;
			if (i<m_polyVect.size()-1)  vals << ", ";
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		child->LinkEndChild( text );
	}
	if(!m_subSurfaces.empty()) {
		TiXmlElement * child = new TiXmlElement("SubSurfaces");
		e->LinkEndChild(child);

		for( const SubSurface& subsurface : m_subSurfaces) {
			subsurface.writeXML(child);
		}
	}
	return e;
}

void Surface::setSurfaceType(IFC4X3::IfcInternalOrExternalEnum::IfcInternalOrExternalEnumEnum type) {
	switch(type) {
		case IFC4X3::IfcInternalOrExternalEnum::ENUM_INTERNAL: m_positionType = Surface::PT_Internal; break;
		case IFC4X3::IfcInternalOrExternalEnum::ENUM_EXTERNAL: m_positionType = Surface::PT_External; break;
		case IFC4X3::IfcInternalOrExternalEnum::ENUM_EXTERNAL_EARTH: m_positionType = Surface::PT_External_Ground; break;
		case IFC4X3::IfcInternalOrExternalEnum::ENUM_EXTERNAL_WATER:
		case IFC4X3::IfcInternalOrExternalEnum::ENUM_EXTERNAL_FIRE:
		case IFC4X3::IfcInternalOrExternalEnum::ENUM_NOTDEFINED: m_positionType = Surface::PT_Unknown; break;
	}
}

bool Surface::isValid() const {
	std::vector<IBKMK::Vector3D> tempVect;
	for(auto v : m_polyVect) {
		if(tempVect.empty())
			tempVect.push_back(v);
		else {
			bool isThere = false;
			for(auto v2 : tempVect) {
				if(nearEqual(v, v2)) {
					isThere = true;
					break;
				}
			}
			if(!isThere)
				tempVect.push_back(v);
		}
	}
	return tempVect.size() > 2;
}

void surfacesFromMeshSets(std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& meshsets, std::vector<Surface>& surfaces) {
	if(meshsets.empty())
		return;

	// try to simplify meshes by merging all coplanar faces
	simplifyMesh(meshsets, false);
	polyVector_t polyvectFinal;
	int msCount = meshsets.size();
	for(int i=0; i<msCount; ++i) {
		polyvectFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
		const carve::mesh::MeshSet<3>& currMeshSet = *meshsets[i];
		convert(currMeshSet, polyvectFinal.back());
		// get surfaces
		for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
			for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
				if(currMeshSet.meshes[mi]->faces[fi] != nullptr)
					surfaces.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
			}
		}
	}
}

void surfacesFromRepresentation(std::shared_ptr<ProductShapeData> productShape, std::vector<Surface>& surfaces,
								std::vector<ConvertError>& errors, ObjectType objectType, int objectId) {

	surfaces.clear();

	int repCount = productShape->m_vec_representations.size();
	std::shared_ptr<RepresentationData> currentRep;
	std::shared_ptr<RepresentationData> bodyRep;
	int bodyRepCount = 0;
	std::shared_ptr<RepresentationData> referenceRep;
	int referenceRepCount = 0;
	std::shared_ptr<RepresentationData> surfaceRep;
	int surfaceRepCount = 0;
	std::shared_ptr<RepresentationData> profileRep;
	int profileRepCount = 0;
	for(int repi = 0; repi<repCount; ++repi) {
		currentRep = productShape->m_vec_representations[repi];
		if(currentRep->m_representation_identifier == "Body") {
			bodyRep = currentRep;
			++bodyRepCount;
		}
		if(currentRep->m_representation_identifier == "Reference") {
			referenceRep = currentRep;
			++referenceRepCount;
		}
		if(currentRep->m_representation_identifier == "Surface") {
			surfaceRep = currentRep;
			++surfaceRepCount;
		}
		if(currentRep->m_representation_identifier == "Profile") {
			profileRep = currentRep;
			++profileRepCount;
		}
	}

	if(bodyRep) {
		if(bodyRepCount > 1) {
			errors.push_back({objectType, objectId, "more than one geometric representaion of type 'body' found"});
		}
		meshVector_t meshSetClosedFinal;
		meshVector_t meshSetOpenFinal;
		for(const auto& shapeData : bodyRep->m_vec_item_data) {
			const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mc = shapeData->m_meshsets;
			if(!mc.empty()) {
				for(auto mSet : mc)
					if(mSet.get() != nullptr) {
						meshSetClosedFinal.push_back(mSet);
					}
					else {
						errors.push_back({objectType, objectId, "Non valid mesh set found."});
					}
			}
			const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mo = shapeData->m_meshsets_open;
			if(!mo.empty()) {
				for(auto mSet : mo)
					if(mSet.get() != nullptr) {
						meshSetOpenFinal.push_back(mSet);
					}
					else {
						errors.push_back({objectType, objectId, "Non valid mesh set found."});
					}
			}
		}

		surfacesFromMeshSets(meshSetClosedFinal, surfaces);
		surfacesFromMeshSets(meshSetOpenFinal, surfaces);
	}

	if(referenceRep) {
		///< \todo Implement
		if(!bodyRep)
			errors.push_back({objectType, objectId, "Geometric representation of type 'reference' cannot be evaluated."});
	}

	if(surfaceRep) {
		///< \todo Implement
		if(!bodyRep)
			errors.push_back({objectType, objectId, "Geometric representation of type 'surface' cannot be evaluated."});
	}

	if(profileRep) {
		///< \todo Implement
		if(!bodyRep)
			errors.push_back({objectType, objectId, "Geometric representation of type 'profile' cannot be evaluated."});
	}

}

static std::shared_ptr<RepresentationData> firstBodyRep(std::shared_ptr<ProductShapeData> productShape) {
	for(auto rep : productShape->m_vec_representations) {
		if(rep->m_representation_identifier == "Body") {
			return rep;
		}
	}

	return std::shared_ptr<RepresentationData>();
}

meshVector_t meshSetsFromBodyRepresentation(std::shared_ptr<ProductShapeData> productShape) {

	std::shared_ptr<RepresentationData> bodyRep = firstBodyRep(productShape);
	if(!bodyRep)
		return meshVector_t();

	meshVector_t meshSetClosedFinal;
	meshVector_t meshSetOpenFinal;
	for(const auto& shapeData : bodyRep->m_vec_item_data) {
		const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mc = shapeData->m_meshsets;
		if(!mc.empty()) {
			meshSetClosedFinal.insert(meshSetClosedFinal.begin(), mc.begin(), mc.end());
		}
		const std::vector<shared_ptr<carve::mesh::MeshSet<3> > >& mo = shapeData->m_meshsets_open;
		if(!mo.empty()) {
			meshSetOpenFinal.insert(meshSetOpenFinal.begin(), mo.begin(), mo.end());
		}
	}

	if(!meshSetClosedFinal.empty()) {
		return meshSetClosedFinal;
	}

	if(!meshSetOpenFinal.empty()) {
		return meshSetOpenFinal;
	}
	return meshVector_t();
}


} // namespace IFCC
