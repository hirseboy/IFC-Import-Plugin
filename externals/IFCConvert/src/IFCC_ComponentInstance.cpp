#include "IFCC_ComponentInstance.h"

#include <tinyxml.h>

#include <IBK_StringUtils.h>

namespace IFCC {

ComponentInstance::ComponentInstance() :
	m_id(-1),
	m_componentId(-1),
	m_sideASurfaceId(-1),
	m_sideBSurfaceId(-1),
	m_subSurface(false)
{}

ComponentInstance::ComponentInstance(int id, int componentId, int sideASurfaceId, int sideBSurfaceId) :
	m_id(id),
	m_componentId(componentId),
	m_sideASurfaceId(sideASurfaceId),
	m_sideBSurfaceId(sideBSurfaceId)
{}

TiXmlElement * ComponentInstance::writeXML(TiXmlElement * parent) const {
	std::string name = m_subSurface ? "SubSurfaceComponentInstance" : "ComponentInstance";
	std::string nameId = m_subSurface ? "idSubSurfaceComponent" : "idComponent";
	TiXmlElement * e = new TiXmlElement(name);
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_componentId != -1)
		e->SetAttribute(nameId, IBK::val2string<unsigned int>(m_componentId));
	if (m_sideASurfaceId != -1)
		e->SetAttribute("idSideASurface", IBK::val2string<unsigned int>(m_sideASurfaceId));
	if (m_sideBSurfaceId != -1)
		e->SetAttribute("idSideBSurface", IBK::val2string<unsigned int>(m_sideBSurfaceId));


	return e;
}

int ComponentInstance::id() const {
	return m_id;
}

bool ComponentInstance::isSubSurface() const {
	return m_subSurface;
}

void ComponentInstance::setSubSurface(bool subSurface) {
	m_subSurface = subSurface;
}

int ComponentInstance::sideASurfaceId() const {
	return m_sideASurfaceId;
}

} // namespace IFCC
