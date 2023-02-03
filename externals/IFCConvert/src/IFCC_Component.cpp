#include "IFCC_Component.h"

#include <IBK_math.h>

#include <tinyxml.h>

#include "IFCC_Surface.h"
#include "IFCC_SpaceBoundary.h"

namespace IFCC {

Component::Component() :
	m_id(-1),
	m_constructionId(-1),
	m_type(NUM_CT),
	m_basictype(NUM_BT)
{

}

TiXmlElement * Component::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Component");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);

	if (!m_notes.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes);
	if (!m_dataSource.empty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource);
	if (!m_manufacturer.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Manufacturer", nullptr, std::string(), m_manufacturer);
	if (m_type != NUM_CT)
		TiXmlElement::appendSingleAttributeElement(e, "Type", nullptr, std::string(), type2String(m_type));
	if (m_constructionId != -1)
		TiXmlElement::appendSingleAttributeElement(e, "IdConstruction", nullptr, std::string(), IBK::val2string<unsigned int>(m_constructionId));


	return e;
}

void Component::updateComponentType(const Surface& surf) {
	if(m_type == CT_OutsideWall) {
		if(surf.positionType() == Surface::PT_Internal)
			m_type = CT_InsideWall;
		else if(surf.positionType() == Surface::PT_External_Ground)
			m_type = CT_OutsideWallToGround;
	}
	else if(m_type == CT_SlopedRoof) {
		// nothing to do
	}
	else if(m_type == CT_FloorToAir) {
		if(surf.positionType() == Surface::PT_Internal)
			m_type = CT_Ceiling;
		else if(surf.positionType() == Surface::PT_External_Ground)
			m_type = CT_FloorToGround;
	}
	else if(m_type == NUM_CT) {
		m_type = CT_Miscellaneous;
	}
}

void Component::updateComponentType(const SpaceBoundary& sb) {
	if(m_type == CT_OutsideWall) {
		if(sb.isInternal())
			m_type = CT_InsideWall;
		else if(sb.isExternalToGround())
			m_type = CT_OutsideWallToGround;
	}
	else if(m_type == CT_SlopedRoof) {
		// nothing to do
	}
	else if(m_type == CT_FloorToAir) {
		if(sb.isInternal())
			m_type = CT_Ceiling;
		else if(sb.isExternalToGround())
			m_type = CT_FloorToGround;
	}
	else if(m_type == NUM_CT) {
		m_type = CT_Miscellaneous;
	}
}

//VICUS::Component Component::getVicusObject(std::map<int,int>& idMap, int idOffset) const {
//	VICUS::Component vcomp;
//	vcomp.m_id = m_id + idOffset;
//	idMap[m_id] = vcomp.m_id;

//	vcomp.m_displayName.setString(m_name,"de");
//	vcomp.m_dataSource.setString(m_dataSource,"de");
//	vcomp.m_manufacturer.setString(m_manufacturer,"de");
//	vcomp.m_notes.setString(m_notes,"de");
//	vcomp.m_idConstruction = idMap[m_constructionId];
//	vcomp.m_type = static_cast<VICUS::Component::ComponentType>(m_type);

//	return vcomp;
//}

std::string Component::type2String(ComponentType type) const {
	switch(type) {
		case CT_OutsideWall: return "OutsideWall";
		case CT_OutsideWallToGround: return "OutsideWallToGround";
		case CT_InsideWall: return "InsideWall";
		case CT_FloorToCellar: return "FloorToCellar";
		case CT_FloorToAir: return "FloorToAir";
		case CT_FloorToGround: return "FloorToGround";
		case CT_Ceiling: return "Ceiling";
		case CT_SlopedRoof: return "SlopedRoof";
		case CT_FlatRoof: return "FlatRoof";
		case CT_ColdRoof: return "ColdRoof";
		case CT_WarmRoof: return "WarmRoof";
		case CT_Miscellaneous: return "Miscellaneous";
		case NUM_CT: return "Unknown";
	}
	return "Unknown";
}


} // namespace IFCC
