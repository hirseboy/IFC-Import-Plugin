#include "IFCC_Instances.h"


//#include <VICUS_Project.h>

#include "IFCC_Helper.h"
#include "IFCC_Site.h"
#include "IFCC_Database.h"

namespace IFCC {

Instances::Instances()
{
}

void Instances::clear() {
	m_componentInstances.clear();
	m_subSurfaceComponentInstances.clear();
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

int Instances::collectComponentInstances(BuildingElementsCollector& elements, Database& database, const Site& site) {
	int res = collectComponentInstances(elements.m_constructionElements, database, site);
	res += collectComponentInstances(elements.m_constructionSimilarElements, database, site);
	res += collectComponentInstances(elements.m_openingElements, database, site);
	return res;
}

int Instances::collectComponentInstances(std::vector<std::shared_ptr<BuildingElement>>& elements, Database& database, const Site& site) {
	std::vector<int> failedSurfaces;
	std::vector<int> failedSubSurfaces;

	for(const auto& building : site.m_buildings) {
		for(const auto& storey : building->storeys()) {
			for(const auto& space : storey->spaces()) {
				for(const auto& surf : space->surfaces()) {
					if(surf.isMissing()) {
						ComponentInstance ci;
						ci.m_id = GUID_maker::instance().guid();
						ci.m_componentId = Database::m_missingComponentId;
						ci.m_sideASurfaceId = surf.id();
						m_componentInstances[ci.m_id] = ci;
					}
					else if(surf.isVirtual()) {
						ComponentInstance ci;
						ci.m_id = GUID_maker::instance().guid();
						ci.m_componentId = Database::m_virtualComponentId;
						ci.m_sideASurfaceId = surf.id();
						m_componentInstances[ci.m_id] = ci;
					}
					else {
						auto fitElem = std::find_if(
										   elements.begin(),
										   elements.end(),
										   [surf](const auto& elem) {return elem->m_id == surf.elementId(); });
						if(fitElem != elements.end()) {
							const std::shared_ptr<BuildingElement>& elem = *fitElem;
							auto fitComp = std::find_if(
											   database.m_components.begin(),
											   database.m_components.end(),
											   [elem](const auto& comp) {return comp.second.m_guid == elem->m_guid; });
							if(fitComp != database.m_components.end()) {
								ComponentInstance ci;
								ci.m_id = GUID_maker::instance().guid();
								ci.m_componentId = fitComp->first;
								ci.m_sideASurfaceId = surf.id();
								fitComp->second.updateComponentType(surf);
								m_componentInstances[ci.m_id] = ci;
							}
							else {
								failedSurfaces.push_back(surf.id());
							}
						}
						else {
							failedSurfaces.push_back(surf.id());
						}
					}
				}

				for(const auto& surf2 : space->surfaces()) {
					for(const auto& subSurf : surf2.subSurfaces()) {
						auto fitElem = std::find_if(
										   elements.begin(),
										   elements.end(),
										   [subSurf](const auto& elem) {return elem->m_id == subSurf.elementId(); });
						if(fitElem != elements.end()) {
							const std::shared_ptr<BuildingElement>& elem = *fitElem;
							auto fitComp = std::find_if(
											   database.m_subSurfaceComponents.begin(),
											   database.m_subSurfaceComponents.end(),
											   [elem](const auto& comp) {return comp.second.guid() == elem->m_guid; });
							if(fitComp != database.m_subSurfaceComponents.end()) {
								ComponentInstance ci;
								ci.m_id = GUID_maker::instance().guid();
								ci.m_subSurface = true;
								ci.m_componentId = fitComp->first;
								ci.m_sideASurfaceId = subSurf.id();
								m_subSurfaceComponentInstances[ci.m_id] = ci;
							}
							else {
								failedSubSurfaces.push_back(subSurf.id());
							}
						}
						else {
							failedSubSurfaces.push_back(subSurf.id());
						}
					}
				}
			}
		}
	}
	return failedSurfaces.size() + failedSubSurfaces.size();
}

//void Instances::addToVicusProject(VICUS::Project* project, std::map<int,int>& idMap) {
//	for(const auto& ci : m_componentInstances) {
//		project->m_componentInstances.emplace_back(ci.second.getVicusComponentInstance(idMap));
//	}
//	for(const auto& sci : m_subSurfaceComponentInstances) {
//		project->m_subSurfaceComponentInstances.emplace_back(sci.second.getVicusSubSurfaceComponentInstance(idMap));
//	}
//}


} // namespace IFCC
