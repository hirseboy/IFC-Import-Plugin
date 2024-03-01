#include "GEGZone.h"

#include <VICUS_ComponentInstance.h>
#include <VICUS_utilities.h>
#include <VICUS_Project.h>

GEGZone::GEGZone(const GEGRoom &room, int id)  :
	m_id(id)
{
	m_rooms.push_back(room);
	update();
	m_name = room.m_name;
}

void GEGZone::update() {
	m_totalANGF = 0;
	m_totalVi = 0;
	m_totalVe = 0;
	m_surfaces.clear();
	for(const auto& room : m_rooms) {
		m_totalANGF += room.m_ANGF;
		m_totalVi += room.m_Vi;
		m_totalVe += room.m_Ve;
		m_surfaces.insert(m_surfaces.end(), room.m_surfaces.begin(), room.m_surfaces.end());
	}
}

bool GEGZone::updateHeating() {
	if(m_rooms.empty()) {
		m_errors << QString("No rooms in GEG zone %1").arg(m_id);
		return false;
	}

	bool heated = m_rooms.front().m_heated;
	m_Ti = m_rooms.front().m_TiSoll;
	if(m_rooms.size() == 1)
		return true;

	for(auto it = m_rooms.begin()+1; it!=m_rooms.end(); ++it) {
		if(it->m_heated != heated) {
			m_errors << QString("Mismatch in heating kind in GEG zone %1").arg(m_id);
			return false;
		}
	}
	return true;
}

QString GEGZone::string() const {
	QString res = m_name;
	res += ";" + QString("%1").arg(m_id);
	res += ";" + QString("%1").arg(m_Ti,0,'f', 0);
	res += ";" + QString("%1").arg(m_Ti,0,'f', 0);
	res += ";" + QString("%1").arg(m_totalANGF,0,'f', 0);
	res += ";" + QString("%1").arg(m_totalVe,0,'f', 0);
	res += ";" + QString("%1").arg(m_totalVi,0,'f', 0);
	res += ";;;;\n";
	return res;
}
