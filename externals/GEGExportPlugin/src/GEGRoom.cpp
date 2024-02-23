#include "GEGRoom.h"

#include <VICUS_Room.h>
#include <VICUS_utilities.h>
#include <VICUS_ZoneTemplate.h>
#include <VICUS_Project.h>

void GEGRoom::set(VICUS::Room &room, const VICUS::Project& project, int& nonValidUsageId) {
	if(room.m_para[VICUS::Room::P_Area].value <= 0)
		room.calculateFloorArea();
	m_ANGF = room.m_para[VICUS::Room::P_Area].value;
	if(room.m_para[VICUS::Room::P_Volume].value <= 0)
		room.calculateVolume();
	m_Vi = room.m_para[VICUS::Room::P_Volume].value;;
	m_Ve = room.m_para[VICUS::Room::P_Volume].value;

	if(room.m_idZoneTemplate != VICUS::INVALID_ID) {
		m_zoneTemplateId = room.m_idZoneTemplate;
		const VICUS::ZoneTemplate* zoneTemplate = VICUS::element(project.m_embeddedDB.m_zoneTemplates, room.m_idZoneTemplate);
		if(zoneTemplate != nullptr) {
			m_zoneTemplateName = QString::fromStdString(zoneTemplate->m_displayName.string());
		}
	}
	else {
		m_zoneTemplateId = nonValidUsageId--;
	}
}
