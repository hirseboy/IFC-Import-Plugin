#include "IFCC_SubSurface.h"

#include <limits>
#include <fstream>
#include <sstream>

#include <IBK_math.h>
#include <IBK_assert.h>

#include "IFCC_Clippertools.h"
#include "IFCC_Helper.h"
#include "IFCC_Surface.h"


//#define SubSurface_DUMP

namespace IFCC {

SubSurface::SubSurface(const std::vector<IBKMK::Vector3D>& polygon, const Surface& parentSurface, const IBKMK::Vector3D& nullPos) :
	m_id(-1),
	m_elementEntityId(-1),
	m_openingId(-1),
	m_valid(false),
	m_planeNormal(parentSurface.polygon())
{
	m_valid = m_polyVect.size() > 2;

//	IBKMK::Vector2D p2NullPos =m_planeNormal.convert3DPoint(nullPos);

	for(const IBKMK::Vector3D& vect : polygon) {
		IBKMK::Vector2D p2 = m_planeNormal.convert3DPoint(vect);
		m_polyVect.push_back(p2);
	}

}

TiXmlElement * SubSurface::writeXML(TiXmlElement * parent) const {
	if (m_id == -1)
		return nullptr;

	TiXmlElement * e = new TiXmlElement("SubSurface");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
//	e->SetAttribute("visible", IBK::val2string<bool>(true));

	if(!m_polyVect.empty()) {
		TiXmlElement * child = new TiXmlElement("Polygon2D");
		e->LinkEndChild(child);

		std::stringstream vals;
		for (unsigned int i=0; i<m_polyVect.size(); ++i) {
			vals << m_polyVect[i].m_x << " " << m_polyVect[i].m_y;
			if (i<m_polyVect.size()-1)  vals << ", ";
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		child->LinkEndChild( text );
	}
	return e;
}

} // namespace IFCC
