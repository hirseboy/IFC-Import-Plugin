#include "GEGSurface.h"

#include <VICUS_Surface.h>
#include <VICUS_Component.h>
#include <VICUS_SubSurfaceComponent.h>
#include <VICUS_SubSurfaceComponentInstance.h>
#include <VICUS_Project.h>
#include <VICUS_utilities.h>
#include <VICUS_BoundaryCondition.h>
#include <VICUS_ComponentInstance.h>

GEGConstruction GEGSurface::set(const VICUS::Surface &surface, const VICUS::Project& project) {
	m_inclination = surface.geometry().inclination(0);
	if(!IBK::near_zero(m_inclination)) {
		m_orientation = surface.geometry().orientation(0);
	}
	else {
		m_orientation = -1;
	}
	m_area = surface.geometry().area(2);

	if(surface.m_componentInstance == nullptr) {
		m_errors << QString("No valid component instance for surface %1").arg(surface.m_id);
		return GEGConstruction();
	}

	const VICUS::Component* component = VICUS::element(project.m_embeddedDB.m_components, surface.m_componentInstance->m_idComponent);
	if(component == nullptr) {
		m_errors << QString("No valid component for surface %1 and component instance %2").arg(surface.m_id).arg(surface.m_componentInstance->m_id);
		return GEGConstruction();
	}

	// Bauteiltypen für Dämmwerk
	// FAW
	// FF zur Außenluft
	// FAW Außentür
	// FD Dach
	// FD Decke nach unten
	// zum unbeheizten Raum
	// Trennwand
	// FF zur unbeheizten Zone
	// Innentür gegen unbeheizte Zone
	// Fd Decke nach oben
	// Fz Decke nach unten
	// Kellerwand
	// Fußboden auf Erdreich
	// Kellerdecke;
	m_sideType = sideType(*component);
	switch(component->m_type) {
		case VICUS::Component::CT_OutsideWall:				m_type = "FAW"; break;
		case VICUS::Component::CT_OutsideWallToGround:		m_type = "Fußboden auf Erdreich"; break;
		case VICUS::Component::CT_InsideWall:				m_type = "Trennwand"; break;
		case VICUS::Component::CT_FloorToCellar:			m_type = "Fz Decke nach unten"; break;
		case VICUS::Component::CT_FloorToAir:				m_type = "Fz Decke nach unten"; break;
		case VICUS::Component::CT_FloorToGround:			m_type = "Fußboden auf Erdreich"; break;
		case VICUS::Component::CT_Ceiling:					m_type = "Fd Decke nach oben"; break;
		case VICUS::Component::CT_SlopedRoof:				m_type = "FD Dach"; break;
		case VICUS::Component::CT_FlatRoof:					m_type = "FD Dach"; break;
		case VICUS::Component::CT_ColdRoof:					m_type = "FD Dach"; break;
		case VICUS::Component::CT_WarmRoof:					m_type = "FD Dach"; break;
		case VICUS::Component::CT_Miscellaneous:			m_type = ""; break;
		default:											m_type = ""; break;
	}

	m_boundaryType = BT_None;
	const VICUS::BoundaryCondition*	sideACondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, component->m_idSideABoundaryCondition);
	const VICUS::BoundaryCondition*	sideBCondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, component->m_idSideBBoundaryCondition);
	double ri = 0.13;
	if(sideACondition != nullptr) {
		double h = sideACondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
		if(h > 0) {
			ri = 1.0 / h;
			m_boundaryType = BT_OneSide;
		}
		else {
			m_warnings << QString("Component %1 error in boundary condition side A - no exchange coefficient").arg(component->m_id);
			ri = 1e30;
		}

	}
	double re = 0.13;
	if(sideBCondition != nullptr) {
		double h = sideBCondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
		if(h > 0) {
			re = 1.0 / h;
			m_boundaryType = m_boundaryType == BT_OneSide ? BT_BothSides : BT_OneSide;
		}
		else {
			m_warnings << QString("Component %1 error in boundary condition side B - no exchange coefficient").arg(component->m_id);
			re = 1e30;
		}
	}

	if(m_boundaryType == BT_None) {
		m_errors << QString("Component %1 don't have any boundary condition").arg(component->m_id);
		return GEGConstruction();
	}

	const VICUS::Construction* construction = VICUS::element(project.m_embeddedDB.m_constructions, component->m_idConstruction);

	if(construction == nullptr) {
		m_errors << QString("Component %1 don't have a construction").arg(component->m_id);
		return GEGConstruction();
	}

	GEGConstruction gegconstruction;
	if(!gegconstruction.set(*construction, project)) {
		m_errors << QString("Component %1 have a non valid construction").arg(component->m_id);
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

	if(subsurface.m_subSurfaceComponentInstance == nullptr) {
		m_errors << QString("No valid component instance for subsurface %1").arg(subsurface.m_id);
		return GEGConstruction();
	}

	const VICUS::Component* component = VICUS::element(project.m_embeddedDB.m_components, surface.m_componentInstance->m_idComponent);
	if(component == nullptr) {
		m_errors << QString("No valid component for surface %1 and component instance %2").arg(surface.m_id).arg(surface.m_componentInstance->m_id);
		return GEGConstruction();
	}

	const VICUS::SubSurfaceComponent* subsurfaceComponent = VICUS::element(project.m_embeddedDB.m_subSurfaceComponents, subsurface.m_subSurfaceComponentInstance->m_idSubSurfaceComponent);
	if(component == nullptr) {
		m_errors << QString("No valid component for subsurface %1 and component instance %2").arg(subsurface.m_id).arg(subsurface.m_subSurfaceComponentInstance->m_id);
		return GEGConstruction();
	}


	// Bauteiltypen für Dämmwerk
	// FAW
	// FF zur Außenluft
	// FAW Außentür
	// FD Dach
	// FD Decke nach unten
	// zum unbeheizten Raum
	// Trennwand
	// FF zur unbeheizten Zone
	// Innentür gegen unbeheizte Zone
	// Fd Decke nach oben
	// Fz Decke nach unten
	// Kellerwand
	// Fußboden auf Erdreich
	// Kellerdecke;
	m_sideType = sideType(*component);
	bool isWindow = false;
	switch(subsurfaceComponent->m_type) {
		case VICUS::SubSurfaceComponent::CT_Window:	{
				isWindow = true;
				switch(m_sideType) {
					case ST_Outside:	m_type = "FF zur Außenluft"; break;
					case ST_Ground:		m_type = ""; break;
					case ST_Inside:		m_type = ""; break;
					case ST_Unheated:	m_type = "FF zur unbeheizten Zone"; break;
					case ST_Unknown:	m_type = ""; break;
				}
				break;
			}
		case VICUS::SubSurfaceComponent::CT_Door:	{
				switch(m_sideType) {
					case ST_Outside:	m_type = "FAW Außentür"; break;
					case ST_Ground:		m_type = ""; break;
					case ST_Inside:		m_type = "t"; break;
					case ST_Unheated:	m_type = "Innentür gegen unbeheizte Zone"; break;
					case ST_Unknown:	m_type = ""; break;
				}
				break;
			}
		case VICUS::SubSurfaceComponent::CT_Miscellaneous:	m_type = ""; break;
		default:											m_type = ""; break;
	}

	m_boundaryType = BT_None;
	const VICUS::BoundaryCondition*	sideACondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, subsurfaceComponent->m_idSideABoundaryCondition);
	const VICUS::BoundaryCondition*	sideBCondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, subsurfaceComponent->m_idSideBBoundaryCondition);
	double ri = 0.13;
	if(sideACondition != nullptr) {
		double h = sideACondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
		if(h > 0) {
			ri = 1.0 / h;
			m_boundaryType = BT_OneSide;
		}
		else {
			m_warnings << QString("Component %1 error in boundary condition side A - no exchange coefficient").arg(component->m_id);
			ri = 1e30;
		}

	}
	double re = 0.13;
	if(sideBCondition != nullptr) {
		double h = sideBCondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
		if(h > 0) {
			re = 1.0 / h;
			m_boundaryType = m_boundaryType == BT_OneSide ? BT_BothSides : BT_OneSide;
		}
		else {
			m_warnings << QString("Component %1 error in boundary condition side B - no exchange coefficient").arg(component->m_id);
			re = 1e30;
		}
	}

	if(m_boundaryType == BT_None) {
		m_errors << QString("Component %1 don't have any boundary condition").arg(component->m_id);
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

GEGSurface::SideType GEGSurface::sideType(const VICUS::Component& component) {
	switch(component.m_type) {
		case VICUS::Component::CT_OutsideWall:				return ST_Outside;
		case VICUS::Component::CT_OutsideWallToGround:		return ST_Ground;
		case VICUS::Component::CT_InsideWall:				return ST_Inside;
		case VICUS::Component::CT_FloorToCellar:			return ST_Unheated;
		case VICUS::Component::CT_FloorToAir:				return ST_Outside;
		case VICUS::Component::CT_FloorToGround:			return ST_Ground;
		case VICUS::Component::CT_Ceiling:					return ST_Inside;
		case VICUS::Component::CT_SlopedRoof:				return ST_Outside;
		case VICUS::Component::CT_FlatRoof:					return ST_Outside;
		case VICUS::Component::CT_ColdRoof:					return ST_Outside;
		case VICUS::Component::CT_WarmRoof:					return ST_Outside;
		case VICUS::Component::CT_Miscellaneous:			return ST_Unknown;
		default:											return ST_Unknown;
	}
}
