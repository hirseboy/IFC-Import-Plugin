#ifndef GEGSurfaceH
#define GEGSurfaceH

#include <QString>

#include "GEGConstruction.h"

namespace VICUS {
	class Project;
	class Surface;
}

class GEGSurface {
public:
	GEGSurface(int id = -1) :
		m_id(id)
	{}

	GEGConstruction set(const VICUS::Surface& surface, const VICUS::Project& project);

	int			m_id = -1;;
	int			m_zoneId = -1;
	int			m_otherZoneId = -1;
	int			m_constructionId = -1;
	double		m_area = 0;
	double		m_orientation;
	double		m_inclination;
	double		m_UValue;
	QString		m_type;
	QStringList	m_errors;
};


#endif // GEGSurfaceH
