#include "IFCC_WindowGlazing.h"

namespace IFCC {

WindowGlazing::WindowGlazing() :
	m_id(-1),
	m_modelType(0),
	m_thermalTransmittance(0)
{

}

TiXmlElement * WindowGlazing::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("WindowGlazingSystem");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
	e->SetAttribute("modelType", "Simple");
	if (!m_notes.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes);
	if (!m_dataSource.empty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource);
	if (!m_manufacturer.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Manufacturer", nullptr, std::string(), m_manufacturer);


	return e;
}

} // namespace IFCC
