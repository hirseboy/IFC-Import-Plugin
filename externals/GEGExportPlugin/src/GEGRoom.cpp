#include "GEGRoom.h"

#include <VICUS_Room.h>
#include <VICUS_utilities.h>
#include <VICUS_ZoneTemplate.h>
#include <VICUS_Project.h>
#include <VICUS_Schedule.h>

#include "GEG_Utilities.h"


static const VICUS::Room* getRoom(const VICUS::Project& project, unsigned int id) {
	for( auto& storey : project.m_buildings.front().m_buildingLevels) {
		for( auto& room : storey.m_rooms) {
			if(room.m_id == id)
				return &room;
		}
	}
	return nullptr;
}

bool GEGRoom::set(VICUS::Room &room, const VICUS::Project& project, int& nonValidUsageId) {
	if(room.m_para[VICUS::Room::P_Area].value <= 0)
		room.calculateFloorArea();
	m_ANGF = room.m_para[VICUS::Room::P_Area].value;
	if(room.m_para[VICUS::Room::P_Volume].value <= 0)
		room.calculateVolume();
	m_Vi = room.m_para[VICUS::Room::P_Volume].value;;
	m_Ve = room.m_para[VICUS::Room::P_Volume].value;
	m_zoneId = room.m_id;
	m_name = room.m_displayName;

	m_TiSoll = -99;
	m_TiSollWE = -99;
	m_heated = false;

	if(room.m_idZoneTemplate == VICUS::INVALID_ID) {
		m_zoneTemplateId = nonValidUsageId--;
		m_warnings << QString("Room %1 doesn't contain a valid zone template");
		return false;
	}

	m_zoneTemplateId = room.m_idZoneTemplate;
	const VICUS::ZoneTemplate* zoneTemplate = VICUS::element(project.m_embeddedDB.m_zoneTemplates, room.m_idZoneTemplate);
	if(zoneTemplate == nullptr) {
		m_zoneTemplateId = nonValidUsageId--;
		m_warnings << QString("Room %1 doesn't contain a valid zone template");
		return false;
	}

	m_zoneTemplateName = fromMultiLanguageString(zoneTemplate->m_displayName,"de");

	int idThermostat = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_ControlThermostat];
	const VICUS::ZoneControlThermostat* thermostat = VICUS::element(project.m_embeddedDB.m_zoneControlThermostats, idThermostat);
	int idHeatingCooling = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_IdealHeatingCooling];
	const VICUS::ZoneIdealHeatingCooling* heating = VICUS::element(project.m_embeddedDB.m_zoneIdealHeatingCooling, idHeatingCooling);
	if(thermostat != nullptr && heating != nullptr) {
		if(thermostat->m_idHeatingSetpointSchedule != VICUS::INVALID_ID) {
			const VICUS::Schedule* heatingSchedule = VICUS::element(project.m_embeddedDB.m_schedules, thermostat->m_idHeatingSetpointSchedule);
			if(heatingSchedule != nullptr) {
				double min;
				double max;
				heatingSchedule->calculateMinMax(min, max);
				double heatingLoad = heating->m_para[VICUS::ZoneIdealHeatingCooling::P_HeatingLimit].value;
				if(max >= 19 && heatingLoad > 0) {
					m_TiSoll = max;
					m_TiSollWE = max;
					m_heated = true;
				}
			}
		}
	}

	// check for surface heating
	if(!m_heated) {
		for(const auto& surface : m_surfaces) {
			if(surface.m_componentInstance != nullptr) {
				if(surface.m_componentInstance->m_idSurfaceHeating != VICUS::INVALID_ID &&
						surface.m_componentInstance->m_idSurfaceHeatingControlZone != VICUS::INVALID_ID) {
					const VICUS::Room* room = getRoom(project, surface.m_componentInstance->m_idSurfaceHeatingControlZone);
					if(room != nullptr) {
						const VICUS::ZoneTemplate* zoneTemplate = VICUS::element(project.m_embeddedDB.m_zoneTemplates, room->m_idZoneTemplate);
						int idThermostat = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_ControlThermostat];
						const VICUS::ZoneControlThermostat* thermostat = VICUS::element(project.m_embeddedDB.m_zoneControlThermostats, idThermostat);
						if(thermostat != nullptr && thermostat->m_idHeatingSetpointSchedule != VICUS::INVALID_ID) {
							const VICUS::Schedule* heatingSchedule = VICUS::element(project.m_embeddedDB.m_schedules, thermostat->m_idHeatingSetpointSchedule);
							if(heatingSchedule != nullptr) {
								double min;
								double max;
								heatingSchedule->calculateMinMax(min, max);
								if(max >= 19) {
									m_TiSoll = max;
									m_TiSollWE = max;
									m_heated = true;
								}
							}
						}
					}
				}
			}
		}
	}

	return true;
}
