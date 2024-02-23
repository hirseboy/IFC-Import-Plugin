#ifndef GEGRoomH
#define GEGRoomH

#include <QString>

#include "GEGSurface.h"

namespace VICUS {
	class Room;
	class project;
}

class GEGRoom
{
public:
	GEGRoom(int id) :
		m_id(id)
	{}

	void set(VICUS::Room& room, const VICUS::Project& project, int& nonValidUsageId);

	int						m_id = -1;
	QString					m_name;
	double					m_TiSoll = -99;
	double					m_TiSollWE = -99;
	double					m_ANGF = 0;
	double					m_Vi = 0;
	double					m_Ve = 0;
	int						m_zoneId = -1;
	QString					m_zoneTemplateName;
	int						m_zoneTemplateId = -1;

	std::vector<GEGSurface>	m_surfaces;
};


#endif // GEGRoomH
