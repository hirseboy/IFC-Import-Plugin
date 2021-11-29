#include "IFCC_Instances.h"
#include "IFCC_Helper.h"
#include "IFCC_Site.h"
#include "IFCC_Database.h"

namespace IFCC {

Instances::Instances()
{

}

TiXmlElement * Instances::writeXML(TiXmlElement * parent) const {
	if(!m_componentInstances.empty()) {
		TiXmlElement * child = new TiXmlElement("ComponentInstances");
		parent->LinkEndChild(child);

		for(const auto& comInst : m_componentInstances) {
			comInst.second.writeXML(child);
		}
	}

	if(!m_subSurfaceComponentInstances.empty()) {
		TiXmlElement * child = new TiXmlElement("SubSurfaceComponentInstances");
		parent->LinkEndChild(child);

		for(const auto& comInst : m_subSurfaceComponentInstances) {
			comInst.second.writeXML(child);
		}
	}

	return parent;
}

int Instances::collectComponentInstances(std::vector<BuildingElement>& elements, Database& database, const Site& site) {
	std::vector<int> failedSurfaces;
	std::vector<int> failedSubSurfaces;

	for(const auto& building : site.m_buildings) {
		for(const auto& storey : building.m_storeys) {
			for(const auto& space : storey.m_spaces) {
				for(const auto& surf : space.m_surfaces) {
					auto fitElem = std::find_if(
								   elements.begin(),
								   elements.end(),
								   [surf](const auto& elem) {return elem.m_id == surf.m_elementEntityId; });
					if(fitElem != elements.end()) {
						const BuildingElement& elem = *fitElem;
						auto fitComp = std::find_if(
									   database.m_components.begin(),
									   database.m_components.end(),
									   [elem](const auto& comp) {return comp.second.m_guid == elem.m_guid; });
						if(fitComp != database.m_components.end()) {
							ComponentInstance ci;
							ci.m_id = GUID_maker::instance().guid();
							ci.m_componentId = fitComp->first;
							ci.m_sideASurfaceId = surf.m_id;
							fitComp->second.updateComponentType(surf);
							m_componentInstances[ci.m_id] = ci;
						}
						else {
							failedSurfaces.push_back(surf.m_id);
						}
					}
					else {
						failedSurfaces.push_back(surf.m_id);
					}
				}

				for(const auto& surf2 : space.m_surfaces) {
					for(const auto& subSurf : surf2.m_subSurfaces) {
						auto fitElem = std::find_if(
										   elements.begin(),
										   elements.end(),
										   [subSurf](const auto& elem) {return elem.m_id == subSurf.m_elementEntityId; });
						if(fitElem != elements.end()) {
							const BuildingElement& elem = *fitElem;
							auto fitComp = std::find_if(
											   database.m_subSurfaceComponents.begin(),
											   database.m_subSurfaceComponents.end(),
											   [elem](const auto& comp) {return comp.second.m_guid == elem.m_guid; });
							if(fitComp != database.m_subSurfaceComponents.end()) {
								ComponentInstance ci;
								ci.m_id = GUID_maker::instance().guid();
								ci.m_subSurface = true;
								ci.m_componentId = fitComp->first;
								ci.m_sideASurfaceId = subSurf.m_id;
								m_subSurfaceComponentInstances[ci.m_id] = ci;
							}
							else {
								failedSubSurfaces.push_back(subSurf.m_id);
							}
						}
						else {
							failedSubSurfaces.push_back(subSurf.m_id);
						}
					}
				}
			}
		}
	}
	return failedSurfaces.size() + failedSubSurfaces.size();
}

} // namespace IFCC
