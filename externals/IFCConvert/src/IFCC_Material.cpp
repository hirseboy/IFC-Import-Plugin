#include "IFCC_Material.h"

#include <IBK_StringUtils.h>

namespace IFCC {

Material::Material() :
	m_id(-1)
{
}

TiXmlElement * Material::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Material");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
	if (!m_notes.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes);
	if (!m_manufacturer.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Manufacturer", nullptr, std::string(), m_manufacturer);
	if (!m_dataSource.empty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource);

	TiXmlElement::appendSingleAttributeElement(e, "Category", nullptr, std::string(), "Miscellaneous");

	return e;
}

} // namespace IFCC
