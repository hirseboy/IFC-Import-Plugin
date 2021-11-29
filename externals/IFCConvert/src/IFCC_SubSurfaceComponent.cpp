#include "IFCC_SubSurfaceComponent.h"

namespace IFCC {

SubSurfaceComponent::SubSurfaceComponent() :
	m_id(-1),
	m_type(NUM_CT),
	m_windowId(-1),
	m_constructionId(-1)
{

}

TiXmlElement * SubSurfaceComponent::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("SubSurfaceComponent");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);

	if (m_type != NUM_CT)
		TiXmlElement::appendSingleAttributeElement(e, "Type", nullptr, std::string(), type2String(m_type));
	if (m_windowId != -1)
		TiXmlElement::appendSingleAttributeElement(e, "IdWindow", nullptr, std::string(), IBK::val2string<unsigned int>(m_windowId));
	if (m_constructionId != -1)
		TiXmlElement::appendSingleAttributeElement(e, "IdConstruction", nullptr, std::string(), IBK::val2string<unsigned int>(m_constructionId));


	return e;
}

std::string SubSurfaceComponent::type2String(SubSurfaceComponentType type) const {
	switch(type) {
		case CT_Window: return "Window";
		case CT_Door: return "Door";
		case CT_Miscellaneous: return "Miscellaneous";
		case NUM_CT: return "Unknown";
	}
}

} // namespace IFCC
