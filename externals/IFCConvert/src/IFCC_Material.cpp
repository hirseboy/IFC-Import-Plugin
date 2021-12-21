#include "IFCC_Material.h"

#include <IBK_StringUtils.h>

namespace IFCC {

Material::Material() :
	m_id(-1),
	m_porosity(-1),
	m_density(-1),
	m_upperMyValue(-1),
	m_lowerMyValue(-1),
	m_moistureDiffusivity(-1),
	m_heatCapacity(-1),
	m_thermalConductivity(-1)
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

	if(m_density > 0)
		TiXmlElement::appendIBKParameterElement(e, "Density", m_densityUnit, m_density);
	if(m_heatCapacity > 0)
		TiXmlElement::appendIBKParameterElement(e, "HeatCapacity", m_heatCapacityUnit, m_heatCapacity);
	if(m_thermalConductivity > 0)
		TiXmlElement::appendIBKParameterElement(e, "Conductivity", m_thermalConductivityUnit, m_thermalConductivity);
	if(m_upperMyValue > 0)
		TiXmlElement::appendIBKParameterElement(e, "Mu", m_upperMyValueUnit, m_upperMyValue);
	if(m_porosity > 0)
		TiXmlElement::appendIBKParameterElement(e, "Wsat", m_porosityUnit, m_porosity);

	return e;
}

void Material::setPropertiesFromPropertyMap(const std::map<std::string,std::map<std::string,Property>>& pmap) {
	double value = 0;
	if(getDoubleProperty(pmap,"Pset_MaterialCommon", "Porosity", value))
		m_porosity = value;
	if(getDoubleProperty(pmap,"Pset_MaterialCommon", "MassDensity", value))
		m_density = value;
	if(getDoubleProperty(pmap,"Pset_MaterialHygroscopic", "UpperVaporResistanceFactor", value))
		m_upperMyValue = value;
	if(getDoubleProperty(pmap,"Pset_MaterialHygroscopic", "LowerVaporResistanceFactor", value))
		m_lowerMyValue = value;
	if(getDoubleProperty(pmap,"Pset_MaterialHygroscopic", "MoistureDiffusivity", value))
		m_moistureDiffusivity = value;
	if(getDoubleProperty(pmap,"Pset_MaterialThermal", "SpecificHeatCapacity", value))
		m_heatCapacity = value;
	if(getDoubleProperty(pmap,"Pset_MaterialThermal", "ThermalConductivity", value))
		m_thermalConductivity = value;
}


} // namespace IFCC
