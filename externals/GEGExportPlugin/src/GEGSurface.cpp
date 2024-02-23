#include "GEGSurface.h"

#include <VICUS_Surface.h>
#include <VICUS_Component.h>
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

	if(surface.m_componentInstance != nullptr) {
		const VICUS::Component* component = VICUS::element(project.m_embeddedDB.m_components, surface.m_componentInstance->m_idComponent);
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
		if(component != nullptr) {
			const VICUS::BoundaryCondition*	sideACondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, component->m_idSideABoundaryCondition);
			const VICUS::BoundaryCondition*	sideBCondition = VICUS::element(project.m_embeddedDB.m_boundaryConditions, component->m_idSideBBoundaryCondition);
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

			const VICUS::Construction* construction = VICUS::element(project.m_embeddedDB.m_constructions, component->m_idConstruction);
			if(construction != nullptr) {
				GEGConstruction gegconstruction;
				gegconstruction.set(*construction, project);

				bool bothWrong = true;
				double ri = 0.13;
				if(sideACondition != nullptr) {
					bothWrong = false;
					double h = sideACondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
					if(h > 0)
						ri = 1.0 / h;
					else
						ri = 1e30;
				}
				double re = 0.13;
				if(sideBCondition != nullptr) {
					bothWrong = false;
					double h = sideBCondition->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
					if(h > 0)
						re = 1.0 / h;
					else
						re = 1e30;
				}
				m_UValue = gegconstruction.calculateUValue(ri, re);
				if(bothWrong) {
					m_errors << QString("Component %1 don't have any boundary conditions").arg(component->m_id);
				}
				return gegconstruction;
			}
		}
		else {
			m_errors << QString("No valid component for surface %1 and component instance %2").arg(surface.m_id).arg(surface.m_componentInstance->m_id);
		}
	}
	else {
		m_errors << QString("No valid component instance for surface %1").arg(surface.m_id);
	}

	return GEGConstruction();
}
