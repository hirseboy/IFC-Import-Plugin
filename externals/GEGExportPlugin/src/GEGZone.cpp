#include "GEGZone.h"

GEGZone::GEGZone(const GEGRoom &room, int id) {
	m_totalANGF = room.m_ANGF;
	m_totalVi = room.m_Vi;
	m_totalVe = room.m_Vi;
	m_name = room.m_name;
	m_rooms.push_back(room);
	m_surfaces.insert(m_surfaces.begin(), room.m_surfaces.begin(), room.m_surfaces.end());
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
