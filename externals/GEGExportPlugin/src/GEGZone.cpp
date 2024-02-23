#include "GEGZone.h"

void GEGZone::update() {
	m_totalANGF = 0;
	m_totalVi = 0;
	m_totalVe = 0;
	for(const auto& room : m_rooms) {
		m_totalANGF += room.m_ANGF;
		m_totalVi += room.m_Vi;
		m_totalVe += room.m_Ve;
	}
}
