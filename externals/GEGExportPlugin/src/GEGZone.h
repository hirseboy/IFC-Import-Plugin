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

	bool updateHeating();

	QString string() const;

	int						m_id = -1;
	QString					m_name;
	double					m_totalANGF = 0;
	double					m_totalVi = 0;
	double					m_totalVe = 0;
	double					m_Ti	  = 0;
	bool					m_isHeated = true;
	std::vector<GEGRoom>	m_rooms;
	std::vector<GEGSurface>	m_surfaces;
	QStringList				m_errors;
	QStringList				m_warnings;
};

#endif // GEGZoneH
