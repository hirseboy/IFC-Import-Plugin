#include "IFCC_Surface.h"

#include <limits>
#include <fstream>
#include <sstream>

#include <IBK_math.h>
#include <IBK_assert.h>

#include "IFCC_Clippertools.h"
#include "IFCC_Helper.h"

//#define SURFACE_DUMP

namespace IFCC {

Surface::Surface() :
	m_id(-1),
	m_elementEntityId(-1),
	m_openingId(-1),
	m_valid(false),
	m_positionType(PT_Unknown),
	m_virtualSurface(false)
{
}

Surface::Surface(carve::mesh::Face<3>* face) :
	m_id(-1),
	m_elementEntityId(-1),
	m_openingId(-1),
	m_valid(false),
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
	m_valid = m_polyVect.size() > 2;
	PlaneHesseNormal hesse(face->plane);
	m_planeNormal = PlaneNormal(hesse, m_polyVect);

	for(const IBKMK::Vector3D& vect : m_polyVect) {
		IBKMK::Vector2D p2 = m_planeNormal.convert3DPoint(vect);
		m_polyVect2D.push_back(p2);
	}

}

Surface::Surface(const polygon3D_t& polygon) :
	m_id(-1),
	m_elementEntityId(-1),
	m_openingId(-1),
	m_valid(false),
	m_positionType(PT_Unknown),
	m_virtualSurface(false),
	m_polyVect(polygon)
{
	m_valid = m_polyVect.size() > 2;

	if(m_valid) {
		m_planeNormal = PlaneNormal(polygon);
		PlaneHesseNormal planeHesseNormal(polygon);
		m_planeCarve.N = carve::geom::VECTOR(planeHesseNormal.m_n0.m_x,planeHesseNormal.m_n0.m_x,planeHesseNormal.m_n0.m_x);
		m_planeCarve.d = planeHesseNormal.m_d;

		for(const IBKMK::Vector3D& vect : m_polyVect) {
			IBKMK::Vector2D p2 = m_planeNormal.convert3DPoint(vect);
			m_polyVect2D.push_back(p2);
		}
	}
}

IBKMK::Vector3D Surface::centroid() const {
	IBKMK::Vector3D res;
	for(const IBKMK::Vector3D& vert : m_polyVect) {
		res += vert;
	}
	return res * (1.0/m_polyVect.size());
}

double Surface::distancePointToPlane(const IBKMK::Vector3D& point, bool negate) const {
	IBKMK::Vector3D n0(m_planeCarve.N.x, m_planeCarve.N.y, m_planeCarve.N.z);
	if(negate)
		n0 = n0*-1;
	double d1 = point.scalarProduct(n0);
	double dist = d1 - m_planeCarve.d;
	return dist;
}

double Surface::distanceToParallelPlane(const carve::geom::plane<3>& other) const {
	double negFact = 1.0;
	if(!nearEqual(m_planeCarve.N,other.N)) {
		if(!nearEqual(m_planeCarve.N.negated(),other.N))
			return std::numeric_limits<double>::max();
		else
			negFact = -1.0;
	}

	double dist = std::fabs(other.d - (m_planeCarve.d * negFact));
	return dist;
}

double Surface::distanceToParallelPlane(const PlaneNormal& other) const {
	double negFact = 1.0;
	if(!nearEqual(m_planeNormal.m_lz, other.m_lz)) {
		if(!nearEqual(m_planeNormal.m_lz*-1,other.m_lz))
			return std::numeric_limits<double>::max();
		else
			negFact = -1.0;
	}

	double dist = std::fabs(other.m_distance - (m_planeNormal.m_distance * negFact));
	return dist;
}

bool Surface::equalNormals(const Surface& other)  const {
	carve::geom::vector<3> v1 = m_planeCarve.N;
	carve::geom::vector<3> v2 = other.planeCarve().N;

	return nearEqual(v1,v2);
}

bool Surface::equalNegNormals(const Surface& other)  const {
	carve::geom::vector<3> v1 = m_planeCarve.N;
	carve::geom::vector<3> v2 = other.planeCarve().N;
	carve::geom::vector<3> negV1 = v1.negated();

	if(!IBK::near_equal(negV1.x,v2.x))
		return false;
	if(!IBK::near_equal(negV1.y,v2.y))
		return false;
	if(!IBK::near_equal(negV1.z,v2.z))
		return false;

	return true;
}

bool Surface::isParallelTo(const Surface& other) const {
	double a;
	double b;
	double c;

	carve::geom::vector<3> v1 = m_planeCarve.N;
	carve::geom::vector<3> v2 = other.planeCarve().N;

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

void Surface::simplify() {
	if(m_polyVect.size() <= 4)
		return;

	std::vector<std::pair<size_t,size_t>> equalPoints;
	for(size_t i=0; i<m_polyVect.size()-1; ++i) {
		for(size_t j=i+1; j<m_polyVect.size(); ++j) {
			if(m_polyVect[i].distanceTo(m_polyVect[j]) < 1e-6)
				equalPoints.push_back(std::make_pair(i,j));
		}
	}
	if(equalPoints.empty())
		return;

	m_polyVect.erase(m_polyVect.begin() + equalPoints.front().second, m_polyVect.end());
}

std::vector<std::pair<size_t,size_t>> Surface::samePoints(const Surface& other) const {
	std::vector<std::pair<size_t,size_t>> equalPoints;
	const std::vector<IBKMK::Vector3D>& otherPoly = other.polygon();
	for(size_t i=0; i<m_polyVect.size(); ++i) {
		for(size_t j=0; j<otherPoly.size(); ++j) {
			if(m_polyVect[i] == otherPoly[j])
				equalPoints.push_back(std::make_pair(i,j));
		}
	}
	return equalPoints;
}

//static bool compare(const polygon3D_t& left, const polygon3D_t& right) {
//	if(left.size() != right.size())
//		return false;

//	for(size_t i=0; i<left.size(); ++i) {
//		double dist = left[i].distanceTo(right[i]);
//		if(dist > 1e-5)
//			return false;
//	}
//	return true;
//}

bool Surface::isIntersected(const Surface& other) const {
	polygon3D_t result = intersectPolygons(m_polyVect, other.polygon(), m_planeNormal);
	if(result.empty() || areaPolygon(result) < 1e-4)
		return false;
	return true;
//	return intersects(m_polyVectOrg, other.polygon());
}

Surface Surface::intersect(const Surface& other) {
	polygon3D_t result = intersectPolygons(m_polyVect, other.polygon(), m_planeNormal);
	if(result.empty() || areaPolygon(result) < 1e-4)
		return Surface();

	return Surface(result);
}

Surface::IntersectionResult Surface::intersect2(const Surface& other) {
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


bool Surface::merge(const Surface& subsurface) {
	polygon3D_t result = mergePolygons(m_polyVect, subsurface.polygon(), m_planeNormal);
	if(result.empty())
		return false;
	m_polyVect = result;
	for(const IBKMK::Vector3D& vect : m_polyVect) {
		IBKMK::Vector2D p2 = m_planeNormal.convert3DPoint(vect);
		m_polyVect2D.push_back(p2);
	}
	return true;
}

bool Surface::addSubSurface(const Surface& subsurface) {
//	PlaneNormal subplane = m_planeNormal;
	m_subSurfaces.emplace_back(SubSurface(subsurface.polygon(), *this, m_polyVect[0]));
	m_subSurfaces.back().m_id = GUID_maker::instance().guid();
	m_subSurfaces.back().m_name = subsurface.m_name;
	m_subSurfaces.back().m_openingId = subsurface.m_openingId;
	m_subSurfaces.back().m_elementEntityId = subsurface.m_elementEntityId;
	return m_subSurfaces.back().m_valid;
}


bool Surface::check() {
#ifdef SURFACE_DUMP
	std::ofstream debugOut("g:/temp/surfaces.txt", std::ios::app);
	std::stringstream debug;

	debug << "\n|pos|: " << m_planeNormal.m_pos.magnitude() << "\n";
	debug << "pos: " << m_planeNormal.m_pos.m_x << "|" << m_planeNormal.m_pos.m_y << "|" << m_planeNormal.m_pos.m_z << "\n";
	debug << "rotPos: " << m_planeNormal.m_rot_pos.m_x << "|" << m_planeNormal.m_rot_pos.m_y << "|" << m_planeNormal.m_rot_pos.m_z << "\n";
	debug << "lx: " << m_planeNormal.m_lx.m_x << "|" << m_planeNormal.m_lx.m_y << "|" << m_planeNormal.m_lx.m_z << "\n";
	debug << "ly: " << m_planeNormal.m_ly.m_x << "|" << m_planeNormal.m_ly.m_y << "|" << m_planeNormal.m_ly.m_z << "\n";
	debug << "lz: " << m_planeNormal.m_lz.m_x << "|" << m_planeNormal.m_lz.m_y << "|" << m_planeNormal.m_lz.m_z << "\n";
	debug << "n0: " << hesse.m_n0.m_x << "|" << hesse.m_n0.m_y << "|" << hesse.m_n0.m_z << "\n";
	debug << "d: " << hesse.m_d << "\n";
	debug << "type: " << m_planeNormal.m_polygonPlane << "\n";
	debug << "area: " << areaPolygon(m_polyVect) << "\n";
	debug << "\nconverted\n";
#endif

	//	PlaneNormal plane2D = PlaneNormal::createXYPlane();
	bool errorInSurface = false;
	IBK_ASSERT(m_polyVect.size() == m_polyVect2D.size());

	for(size_t i=0; i<m_polyVect.size(); ++i) {
		const IBKMK::Vector2D& p2 = m_polyVect2D[i];
		const IBKMK::Vector3D& p3 = m_polyVect[i];
		IBKMK::Vector3D p3Check = m_planeNormal.convert3DPointInv(p2);
		errorInSurface = !nearEqual(p3Check, p3);

#ifdef SURFACE_DUMP
		debug << "p2 " << p2.m_x << "|" << p2.m_y << "|" << 0 << "\n";

		debug << "org " << p3.m_x << "|" << p3.m_y << "|" << p3.m_z << "   " << "\n";
		debug << "p3C " << p3Check.m_x << "|" << p3Check.m_y << "|" << p3Check.m_z << "   " << nearEqual(p3Check, vect) << "\n";
		if(p3Check != vect) {
			m_valid = false;
		}
#endif
	}
#ifdef SURFACE_DUMP
	if(errorInSurface) {
		debugOut << debug.str();
	}
	else {
//		debugOut << "no errors in surface\n";
	}
#endif
	return !errorInSurface;
}

double Surface::area() const {
	return areaPolygon(m_polyVect);
}

Surface::MatchResult Surface::findFirstSurfaceMatchIndex(const std::vector<Surface>& wallSurfaces, const std::vector<Surface>& spaceSurfaces, double minDist) {
	for(size_t wi=0; wi<wallSurfaces.size(); ++wi) {
		for(size_t si=0; si<spaceSurfaces.size(); ++si) {
			double dist = spaceSurfaces[si].distanceToParallelPlane(wallSurfaces[wi].planeNormal());
			if(dist < minDist) {
				if(wallSurfaces[wi].isIntersected(spaceSurfaces[si]))
					return MatchResult(wi,si);
			}
		}
	}
	return MatchResult();
}


} // namespace IFCC
