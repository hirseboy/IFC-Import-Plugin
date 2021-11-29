#include "IFCC_Window.h"

namespace IFCC {

Window::Window() :
	m_id(-1),
	m_glazingSystemId(-1),
	m_methodFrame(3),
	m_methodDivider(3)
{

}

TiXmlElement * Window::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Window");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
	if (m_glazingSystemId > -1)
		e->SetAttribute("idGlazingSystem", IBK::val2string<unsigned int>(m_glazingSystemId));
	if (m_methodFrame < 3)
		e->SetAttribute("methodFrame", IBK::val2string<unsigned int>(m_methodFrame));
	if (m_methodDivider < 3)
		e->SetAttribute("methodDivider", IBK::val2string<unsigned int>(m_methodDivider));
	if (!m_notes.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes);
	if (!m_dataSource.empty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource);


	return e;
}

} // namespace IFCC
