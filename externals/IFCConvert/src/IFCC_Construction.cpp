#include "IFCC_Construction.h"

namespace IFCC {

Construction::Construction()
{

}

TiXmlElement * Construction::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Construction");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);

	if(!m_layers.empty()) {
		TiXmlElement * child = new TiXmlElement("MaterialLayers");
		e->LinkEndChild(child);

		for( const auto& layer : m_layers) {
			TiXmlElement * childLayer = new TiXmlElement("MaterialLayer");
			child->LinkEndChild(childLayer);

			childLayer->SetAttribute("idMaterial", IBK::val2string<unsigned int>(layer.first));
			TiXmlElement::appendIBKParameterElement(childLayer, "Thickness", "m", layer.second);
		}
	}

	return e;
}

} // namespace IFCC
