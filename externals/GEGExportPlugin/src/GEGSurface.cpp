#include "GEGSurface.h"

#include <VICUS_Surface.h>
#include <VICUS_Component.h>
#include <VICUS_SubSurfaceComponent.h>
#include <VICUS_SubSurfaceComponentInstance.h>
#include <VICUS_Project.h>
#include <VICUS_utilities.h>
#include <VICUS_BoundaryCondition.h>
#include <VICUS_ComponentInstance.h>

#include "GEG_Utilities.h"

GEGConstruction GEGSurface::set(const VICUS::Surface &surface, const VICUS::Project& project) {
	m_inclination = surface.geometry().inclination(0);
	if(!IBK::near_zero(m_inclination)) {
		m_orientation = surface.geometry().orientation(0);
	}
	else {
		m_orientation = -1;
	}
	m_area = surface.geometry().area(2);
	m_name =  surface.m_displayName;

	m_componentInstance = surface.m_componentInstance;
	if(m_componentInstance == nullptr) {
		m_errors << QString("No valid component instance for surface %1").arg(surface.m_id);
		return GEGConstruction();
	}

	m_component = VICUS::element(project.m_embeddedDB.m_components, surface.m_componentInstance->m_idComponent);
	if(m_component == nullptr) {
		m_errors << QString("No valid component for surface %1 and component instance %2").arg(surface.m_id).arg(surface.m_componentInstance->m_id);
		return GEGConstruction();
	}
	m_name =  fromMultiLanguageString(m_component->m_displayName, "de");

	m_sideACondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, m_component->m_idSideABoundaryCondition);
	m_sideBCondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, m_component->m_idSideBBoundaryCondition);

	setSideType();
	if(m_sideType == ST_Invalid || m_sideType == ST_Unknown) {
		m_errors << QString("Side settings for surface %1 and component instance %2 not valid").arg(surface.m_id).arg(surface.m_componentInstance->m_id);
		return GEGConstruction();
	}

	m_orgSurfaceId = surface.m_id;
	m_orgOtherSurfaceId = m_componentInstance->m_idSideASurface == m_orgSurfaceId ? m_componentInstance->m_idSideBSurface : m_componentInstance->m_idSideASurface;

	// Bauteiltypen für Dämmwerk
	// FAW
	// FD Dach
	// FD Decke nach unten
	// zum unbeheizten Raum
	// Trennwand
	// Fd Decke nach oben
	// Fz Decke nach unten
	// Kellerwand
	// Fußboden auf Erdreich
	// Kellerdecke;

	// FF zur Außenluft
	// FAW Außentür
	// FF zur unbeheizten Zone
	// Innentür gegen unbeheizte Zone
	switch(m_component->m_type) {
		case VICUS::Component::CT_OutsideWall:				m_type = "FAW"; m_constructionType = CT_Wall; break;
		case VICUS::Component::CT_OutsideWallToGround:		m_type = "Fußboden auf Erdreich"; m_constructionType = CT_Floor; break;
		case VICUS::Component::CT_InsideWall:				m_type = "Trennwand"; m_constructionType = CT_Wall; break;
		case VICUS::Component::CT_FloorToCellar:			m_type = "Fz Decke nach unten"; m_constructionType = CT_Floor; break;
		case VICUS::Component::CT_FloorToAir:				m_type = "Fz Decke nach unten"; m_constructionType = CT_Floor; break;
		case VICUS::Component::CT_FloorToGround:			m_type = "Fußboden auf Erdreich"; m_constructionType = CT_Floor; break;
		case VICUS::Component::CT_Ceiling:					m_type = "Fd Decke nach oben"; m_constructionType = CT_Ceiling; break;
		case VICUS::Component::CT_SlopedRoof:				m_type = "FD Dach"; m_constructionType = CT_Roof; break;
		case VICUS::Component::CT_FlatRoof:					m_type = "FD Dach"; m_constructionType = CT_Roof; break;
		case VICUS::Component::CT_ColdRoof:					m_type = "FD Dach"; m_constructionType = CT_Roof; break;
		case VICUS::Component::CT_WarmRoof:					m_type = "FD Dach"; m_constructionType = CT_Roof; break;
		case VICUS::Component::CT_Miscellaneous:			m_type = ""; break;
		default:											m_type = ""; break;
	}

	m_GEGType = getSurfaceType();

	double ri = 0.13;
	double re = 0.13;
	setBoundaryType(ri, re);

	if(m_boundaryType == BT_None) {
		m_errors << QString("Component %1 don't have any boundary condition").arg(m_component->m_id);
		return GEGConstruction();
	}

	const VICUS::Construction* construction = VICUS::element(project.m_embeddedDB.m_constructions, m_component->m_idConstruction);

	if(construction == nullptr) {
		m_errors << QString("Component %1 don't have a construction").arg(m_component->m_id);
		return GEGConstruction();
	}

	GEGConstruction gegconstruction;
	if(!gegconstruction.set(*construction, project)) {
		m_errors << QString("Component %1 have a non valid construction").arg(m_component->m_id);
		return GEGConstruction();
	}


	m_constructionId = construction->m_id;

	m_UValue = gegconstruction.calculateUValue(ri, re);
	return gegconstruction;

}

GEGConstruction GEGSurface::set(const VICUS::Surface& surface, const VICUS::SubSurface& subsurface, const VICUS::Project &project) {
	m_inclination = surface.geometry().inclination(0);
	if(!IBK::near_zero(m_inclination)) {
		m_orientation = surface.geometry().orientation(0);
	}
	else {
		m_orientation = -1;
	}
	m_area = subsurface.m_polygon2D.area(2);
	m_name =  surface.m_displayName;

	m_componentInstance = surface.m_componentInstance;
	if(m_componentInstance == nullptr) {
		m_errors << QString("No valid component instance for surface %1").arg(surface.m_id);
		return GEGConstruction();
	}

	if(subsurface.m_subSurfaceComponentInstance == nullptr) {
		m_errors << QString("No valid component instance for subsurface %1").arg(subsurface.m_id);
		return GEGConstruction();
	}

	m_component = VICUS::element(project.m_embeddedDB.m_components, surface.m_componentInstance->m_idComponent);
	if(m_component == nullptr) {
		m_errors << QString("No valid component for surface %1 and component instance %2").arg(surface.m_id).arg(surface.m_componentInstance->m_id);
		return GEGConstruction();
	}

	const VICUS::SubSurfaceComponent* subsurfaceComponent = VICUS::element(project.m_embeddedDB.m_subSurfaceComponents, subsurface.m_subSurfaceComponentInstance->m_idSubSurfaceComponent);
	if(subsurfaceComponent == nullptr) {
		m_errors << QString("No valid component for subsurface %1 and component instance %2").arg(subsurface.m_id).arg(subsurface.m_subSurfaceComponentInstance->m_id);
		return GEGConstruction();
	}
	m_name =  fromMultiLanguageString(subsurfaceComponent->m_displayName, "de");

	m_sideACondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, subsurfaceComponent->m_idSideABoundaryCondition);
	m_sideBCondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, subsurfaceComponent->m_idSideBBoundaryCondition);

	setSideType();
	if(m_sideType == ST_Invalid || m_sideType == ST_Unknown) {
		m_errors << QString("Side settings for surface %1 and component instance %2 not valid").arg(surface.m_id).arg(surface.m_componentInstance->m_id);
		return GEGConstruction();
	}

	// Bauteiltypen für Dämmwerk
	// FAW
	// FD Dach
	// FD Decke nach unten
	// zum unbeheizten Raum
	// Trennwand
	// Fd Decke nach oben
	// Fz Decke nach unten
	// Kellerwand
	// Fußboden auf Erdreich
	// Kellerdecke;

	// FF zur Außenluft
	// FAW Außentür
	// FF zur unbeheizten Zone
	// Innentür gegen unbeheizte Zone

	bool isWindow = false;
	switch(subsurfaceComponent->m_type) {
		case VICUS::SubSurfaceComponent::CT_Window:	{
				isWindow = true;
				m_constructionType = CT_Window;
				switch(m_sideType) {
					case ST_Outside:			m_type = "FF zur Außenluft"; break;
					case ST_GroundOrFixed:		m_type = ""; break;
					case ST_Inside:				m_type = ""; break;
					case ST_InsideAdiabatic:	m_type = ""; break;
					case ST_Unheated:			m_type = "FF zur unbeheizten Zone"; break;
					case ST_Unknown:			m_type = ""; break;
					case ST_Invalid:			m_type = ""; break;
				}
				break;
			}
		case VICUS::SubSurfaceComponent::CT_Door:	{
				m_constructionType = CT_Door;
				switch(m_sideType) {
					case ST_Outside:			m_type = "FAW Außentür"; break;
					case ST_GroundOrFixed:		m_type = ""; break;
					case ST_Inside:				m_type = ""; break;
					case ST_InsideAdiabatic:	m_type = ""; break;
					case ST_Unheated:			m_type = "Innentür gegen unbeheizte Zone"; break;
					case ST_Unknown:			m_type = ""; break;
					case ST_Invalid:			m_type = ""; break;
				}
				break;
			}
		case VICUS::SubSurfaceComponent::CT_Miscellaneous:	m_type = ""; break;
		default:											m_type = ""; break;
	}

	m_GEGType = getSurfaceType();

	double ri = 0.13;
	double re = 0.13;
	setBoundaryType(ri, re);

	if(m_boundaryType == BT_None) {
		m_errors << QString("Component %1 don't have any boundary condition").arg(m_component->m_id);
		return GEGConstruction();
	}

	if(isWindow) {
		const VICUS::Window* window = VICUS::element(project.m_embeddedDB.m_windows, subsurfaceComponent->m_idWindow);

		if(window == nullptr || !window->isValid()) {
			m_errors << QString("SubsurfacComponent %1 don't have a valid window type").arg(subsurfaceComponent->m_id);
			return GEGConstruction();
		}

		GEGConstruction gegconstruction;
		if(!gegconstruction.set(*window, project)) {
			m_errors << QString("SubsurfacComponent %1 have a non valid window").arg(subsurfaceComponent->m_id);
			m_errors << gegconstruction.m_errors;
			return GEGConstruction();
		}

		m_constructionId = window->m_id;

		m_UValue = gegconstruction.calculateUValue(ri, re);
		return gegconstruction;
	}
	else {

		const VICUS::Construction* construction = VICUS::element(project.m_embeddedDB.m_constructions, subsurfaceComponent->m_idConstruction);

		if(construction == nullptr) {
			m_errors << QString("SubsurfacComponent %1 don't have a construction").arg(subsurfaceComponent->m_id);
			return GEGConstruction();
		}

		GEGConstruction gegconstruction;
		if(!gegconstruction.set(*construction, project)) {
			m_errors << QString("SubsurfacComponent %1 have a non valid construction").arg(subsurfaceComponent->m_id);
			m_errors << gegconstruction.m_errors;
			return GEGConstruction();
		}

		m_constructionId = construction->m_id;

		m_UValue = gegconstruction.calculateUValue(ri, re);
		return gegconstruction;
	}
}

void GEGSurface::setSideType() {
	Q_ASSERT(m_componentInstance != nullptr);
	Q_ASSERT(m_component != nullptr);

	BothSideType bst = bothSideType(*m_componentInstance);
	SideType st1 = sideType(*m_component);
	SideType st2 = sideType(bst, m_sideACondition, m_sideBCondition);
	if(st1 != st2) {
		m_warnings << QString("Component type not equal evaluated type for component instance %1").arg(m_componentInstance->m_id);
	}
	m_sideType = st2;
}

void GEGSurface::setBoundaryType(double &ri, double &re) {
	m_boundaryType = BT_None;
	if(m_sideACondition != nullptr) {
		double h = m_sideACondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
		if(h > 0) {
			ri = 1.0 / h;
			m_boundaryType = BT_OneSide;
		}
		else {
			m_warnings << QString("Component %1 error in boundary condition side A - no exchange coefficient").arg(m_component->m_id);
			ri = 1e30;
		}

	}
	if(m_sideBCondition != nullptr) {
		double h = m_sideBCondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
		if(h > 0) {
			re = 1.0 / h;
			m_boundaryType = m_boundaryType == BT_OneSide ? BT_BothSides : BT_OneSide;
		}
		else {
			m_warnings << QString("Component %1 error in boundary condition side B - no exchange coefficient").arg(m_component->m_id);
			re = 1e30;
		}
	}
}

GEGSurface::GEGSurfaceType GEGSurface::getSurfaceType() const {
	if(m_constructionType == CT_Roof || m_constructionType == CT_Ceiling)
		return GST_UpperBoundary;
	if(m_sideType == ST_Outside && (m_constructionType == CT_Wall || m_constructionType == CT_Door || m_constructionType == CT_Window))
		return GST_OuterSurface;
	if(m_sideType == ST_Inside)
		return GST_InnerSurface;
	if(m_sideType == ST_GroundOrFixed && m_constructionType == CT_Wall)
		return GST_LowerBoundary;
	if(m_constructionType == CT_Floor && m_sideType != ST_GroundOrFixed)
		return GST_LowerBoundary;
	if(m_constructionType == CT_Floor && m_sideType == ST_GroundOrFixed)
		return GST_GroundSlab;
	return GST_Unknown;
}


static QString orientationString(double orientation) {
	if(orientation > 337.5 || orientation <= 22.5)
		return "N";
	if(orientation > 22.5 && orientation <= 67.5)
		return "NO";
	if(orientation > 67.5 && orientation <= 112.5)
		return "O";
	if(orientation > 112.5 && orientation <= 157.5)
		return "SO";
	if(orientation > 157.5 && orientation <= 202.5)
		return "S";
	if(orientation > 202.5 && orientation <= 247.5)
		return "SW";
	if(orientation > 247.5 && orientation <= 292.5)
		return "W";
	if(orientation > 292.5 && orientation <= 337.5)
		return "NW";
	return "N";
}

QString GEGSurface::string() const {
	QString res = m_name;
	res += ";" + QString("%1").arg(m_area,0,'f', 0);
	res += ";" + orientationString(m_orientation);
	res += ";" + QString("%1").arg(m_inclination,0,'f', 0);
	res += ";" + QString("%1").arg(m_zoneId);
	res += ";" + QString("%1").arg(m_otherZoneId);
	res += ";" + QString("%1").arg(m_id);
	res += ";" + QString("%1").arg(m_UValue,0,'f', 0);
	res += ";" + m_type;
	res += ";" + QString("%1").arg(m_constructionId);
	res += ";\n";
	return res;
}

GEGSurface::SideType GEGSurface::sideType(const VICUS::Component& component) {
	switch(component.m_type) {
		case VICUS::Component::CT_OutsideWall:				return ST_Outside;
		case VICUS::Component::CT_OutsideWallToGround:		return ST_GroundOrFixed;
		case VICUS::Component::CT_InsideWall:				return ST_Inside;
		case VICUS::Component::CT_FloorToCellar:			return ST_Unheated;
		case VICUS::Component::CT_FloorToAir:				return ST_Outside;
		case VICUS::Component::CT_FloorToGround:			return ST_GroundOrFixed;
		case VICUS::Component::CT_Ceiling:					return ST_Inside;
		case VICUS::Component::CT_SlopedRoof:				return ST_Outside;
		case VICUS::Component::CT_FlatRoof:					return ST_Outside;
		case VICUS::Component::CT_ColdRoof:					return ST_Outside;
		case VICUS::Component::CT_WarmRoof:					return ST_Outside;
		case VICUS::Component::CT_Miscellaneous:			return ST_Invalid;
		default:											return ST_Invalid;
	}
}

GEGSurface::BothSideType GEGSurface::bothSideType(const VICUS::ComponentInstance &componentInstance) {
	unsigned int sa = componentInstance.m_idSideASurface;
	unsigned int sb = componentInstance.m_idSideBSurface;
	if(sa == VICUS::INVALID_ID) {
		if(sb == VICUS::INVALID_ID)
			return BST_Invalid;
		return BST_SideB;
	}
	if(sb == VICUS::INVALID_ID)
		return BST_SideA;
	return BST_Both;
}

GEGSurface::SideType GEGSurface::sideType(BothSideType bothSideType, const VICUS::BoundaryCondition *sideACondition, const VICUS::BoundaryCondition *sideBCondition) {
	// at least one boundary condition should be exist
	if(sideACondition == nullptr && sideBCondition == nullptr)
		return ST_Invalid;

	// check if at least one side id is set
	if(bothSideType == BST_Invalid)
		return ST_Invalid;

	// if both side ids are set
	if(bothSideType == BST_Both) {
		// and both boundary conditions exist its a inside construction
		if(sideACondition != nullptr && sideBCondition != nullptr)
			return ST_Inside;
		// otherwise its not valid
		return ST_Invalid;
	}

	// side A is an id and side B not
	if(bothSideType == BST_SideA) {
		if(sideACondition == nullptr)
			return ST_Invalid;

		if(sideBCondition == nullptr)
			return ST_InsideAdiabatic;

		if(sideBCondition->m_heatConduction.m_otherZoneType == VICUS::InterfaceHeatConduction::OZ_Standard)
			return ST_Outside;

		return ST_GroundOrFixed;
	}

	// side B is an id and side A not
	if(bothSideType == BST_SideB) {
		if(sideBCondition == nullptr)
			return ST_Invalid;

		if(sideACondition == nullptr)
			return ST_InsideAdiabatic;

		if(sideACondition->m_heatConduction.m_otherZoneType == VICUS::InterfaceHeatConduction::OZ_Standard)
			return ST_Outside;

		return ST_GroundOrFixed;
	}

	return ST_Invalid;
}
