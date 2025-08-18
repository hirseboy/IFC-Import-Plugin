#include "IFCC_SubSurface.h"

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
#include "IFCC_Surface.h"


namespace IFCC {

SubSurface::SubSurface(const std::vector<IBKMK::Vector3D>& polygon, const Surface& parentSurface) :
	m_id(-1),
	m_elementEntityId(-1),
	m_valid(false),
	m_planeNormal(parentSurface.polygon())
{
	m_valid = polygon.size() > 2;

	for(const IBKMK::Vector3D& vect : polygon) {
		IBKMK::Vector2D p2 = m_planeNormal.convert3DPoint(vect);
		m_polyVect.push_back(p2);
	}

}

void SubSurface::set(int id, const std::string& name, int elementId) {
	m_id = id;
	m_name = name;
	m_elementEntityId = elementId;
}

void SubSurface::flip(bool positive) {
	double area = areaSignedPolygon(m_polyVect);
	bool isPositive = area >= 0;
	if((isPositive && !positive) || (!isPositive && positive))
		std::reverse(m_polyVect.begin(), m_polyVect.end());
}


TiXmlElement * SubSurface::writeXML(TiXmlElement * parent, bool isHole) const {
	if (m_id == -1)
		return nullptr;

	std::string text = isHole ? "Hole" : "SubSurface";
	TiXmlElement * e = new TiXmlElement(text);
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name + "_" + std::to_string(m_id));
//	e->SetAttribute("visible", IBK::val2string<bool>(true));

	writeXMLPolygon2D(m_polyVect, e);
	return e;
}

} // namespace IFCC
