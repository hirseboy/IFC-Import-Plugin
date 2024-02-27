#ifndef GEGZoneH
#define GEGZoneH

#include <QString>

#include "GEGRoom.h"
#include "GEGSurface.h"

class GEGZone {
public:
	GEGZone(int id = -1) :
		m_id(id)
	{}

	GEGZone(const GEGRoom& room, int id);

	void update();

	int						m_id = -1;
	QString					m_name;
	std::vector<GEGRoom>	m_rooms;
	double					m_totalANGF = 0;
	double					m_totalVi = 0;
	double					m_totalVe = 0;
	std::vector<GEGSurface>	m_surfaces;
};

#endif // GEGZoneH
