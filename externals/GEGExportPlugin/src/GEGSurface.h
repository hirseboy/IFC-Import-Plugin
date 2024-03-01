#ifndef GEGSurfaceH
#define GEGSurfaceH

#include <QString>

#include <VICUS_Constants.h>

#include "GEGConstruction.h"

namespace VICUS {
	class Project;
	class Surface;
	class SubSurface;
	class Component;
	class ComponentInstance;
	class BoundaryCondition;
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
		ST_GroundOrFixed,
		ST_Inside,
		ST_InsideAdiabatic,
		ST_Unheated,
		ST_Invalid,
		ST_Unknown
	};

	enum BothSideType {
		BST_Both,
		BST_SideA,
		BST_SideB,
		BST_Invalid
	};

	enum GEGSurfaceType {
		GST_UpperBoundary,
		GST_OuterSurface,
		GST_InnerSurface,
		GST_LowerBoundary,
		GST_GroundSlab,
		GST_Unknown
	};

	enum ConstructionType {
		CT_Wall,
		CT_Roof,
		CT_Floor,
		CT_Ceiling,
		CT_Door,
		CT_Window,
		CT_Unknown
	};

	GEGSurface(int id = -1) :
		m_id(id)
	{}

	GEGConstruction set(const VICUS::Surface& surface, const VICUS::Project& project);

	GEGConstruction set(const VICUS::Surface& surface, const VICUS::SubSurface& subsurface, const VICUS::Project& project);

	void setSideType();

	void setBoundaryType(double& ri, double& re);

	GEGSurfaceType getSurfaceType() const;

	QString string() const;

	int				m_id = -1;
	QString			m_name;
	int				m_zoneId = -1;
	int				m_otherZoneId = -1;
	int				m_constructionId = -1;
	unsigned int	m_orgSurfaceId = VICUS::INVALID_ID;
	unsigned int	m_orgOtherSurfaceId = VICUS::INVALID_ID;
	double			m_area = 0;
	double			m_orientation;
	double			m_inclination;
	double			m_UValue;
	QString			m_type;
	QStringList		m_errors;
	QStringList		m_warnings;
	BoundaryType	m_boundaryType = BT_Unknown;
	SideType		m_sideType = ST_Unknown;
	GEGSurfaceType	m_GEGType = GST_Unknown;
	ConstructionType	m_constructionType = CT_Unknown;

	const VICUS::ComponentInstance*		m_componentInstance = nullptr;
	const VICUS::Component*				m_component = nullptr;
	const VICUS::BoundaryCondition*		m_sideACondition = nullptr;
	const VICUS::BoundaryCondition*		m_sideBCondition = nullptr;

private:
	static SideType sideType(const VICUS::Component& component);
	static SideType sideType(BothSideType bothSideType, const VICUS::BoundaryCondition*	sideACondition, const VICUS::BoundaryCondition*	sideBCondition);
	static BothSideType bothSideType(const VICUS::ComponentInstance& componentInstance);
};


#endif // GEGSurfaceH
