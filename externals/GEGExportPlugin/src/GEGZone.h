#ifndef GEGZoneH
#define GEGZoneH

#include <QString>

class GEGZone
{
public:
	GEGZone();

	int		m_id;
	QString	m_name;
	double	m_TiSoll;
	double	m_TiSollWE;
	double	m_ANGF;
	double	m_Vi;
	double	m_Ve;
	int		m_zoneId;
};

#endif // GEGZoneH
