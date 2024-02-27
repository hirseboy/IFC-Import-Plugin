#ifndef GEGSurfaceH
#define GEGSurfaceH

#include <QString>

#include "GEGConstruction.h"

namespace VICUS {
	class Project;
	class Surface;
	class SubSurface;
	class Component;
}

class GEGSurface {
public:
	enum BoundaryType {
		BT_BothSides,
		BT_OneSide,
		BT_None,
		BT_Unknown
	};

	enum SideType {
		ST_Outside,
		ST_Ground,
		ST_Inside,
		ST_Unheated,
		ST_Unknown
	};

	GEGSurface(int id = -1) :
		m_id(id)
	{}

	GEGConstruction set(const VICUS::Surface& surface, const VICUS::Project& project);

	GEGConstruction set(const VICUS::Surface& surface, const VICUS::SubSurface& subsurface, const VICUS::Project& project);

	int				m_id = -1;;
	int				m_zoneId = -1;
	int				m_otherZoneId = -1;
	int				m_constructionId = -1;
	double			m_area = 0;
	double			m_orientation;
	double			m_inclination;
	double			m_UValue;
	QString			m_type;
	QStringList		m_errors;
	QStringList		m_warnings;
	BoundaryType	m_boundaryType = BT_Unknown;
	SideType		m_sideType = ST_Unknown;

private:
	static SideType sideType(const VICUS::Component& component);
};


#endif // GEGSurfaceH
