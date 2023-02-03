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

void Instances::collectComponentInstances(BuildingElementsCollector& elements, Database& database, const Site& site,
										 std::vector<ConvertError>& errors) {
	collectNormalComponentInstances(elements, database, site, errors);
	collectSubSurfaceComponentInstances(elements, database, site, errors);
}

void Instances::collectNormalComponentInstances(BuildingElementsCollector& elements, Database& database,
										 const Site& site, std::vector<ConvertError>& errors) {
	std::vector<std::shared_ptr<BuildingElement>> constructionElements = elements.allConstructionElements();
	for(const auto& building : site.m_buildings) {
		for(const auto& storey : building->storeys()) {
			for(const auto& space : storey->spaces()) {
				for(const auto& sb : space->spaceBoundaries()) {
					// first loop is only for normal component instances
					if(sb->isOpeningElement())
						continue;

					// don't go further if the space boundary is already assigned
					if(sb->m_componentInstanceId > -1)
						continue;

					if(sb->isMissing()) {
						ComponentInstance ci;
						ci.m_id = GUID_maker::instance().guid();
						ci.m_componentId = Database::m_missingComponentId;
						ci.m_sideASurfaceId = sb->surface().id();
						m_componentInstances[ci.m_id] = ci;
						sb->m_componentInstanceId = ci.m_id;
					}
					else if(sb->isVirtual()) {
						ComponentInstance ci;
						ci.m_id = GUID_maker::instance().guid();
						ci.m_componentId = Database::m_virtualComponentId;
						ci.m_sideASurfaceId = sb->surface().id();
						m_componentInstances[ci.m_id] = ci;
						sb->m_componentInstanceId = ci.m_id;
					}
					else {
						auto fitElem = std::find_if(
										   constructionElements.begin(),
										   constructionElements.end(),
										   [sb](const auto& elem) {return elem->m_id == sb->m_elementEntityId; });
						if(fitElem != constructionElements.end()) {
							const std::shared_ptr<BuildingElement>& elem = *fitElem;
							auto fitComp = std::find_if(
											   database.m_components.begin(),
											   database.m_components.end(),
											   [elem](const auto& comp) {return comp.second.m_guid == elem->m_guid; });
							if(fitComp != database.m_components.end()) {
								ComponentInstance ci;
								ci.m_id = GUID_maker::instance().guid();
								ci.m_componentId = fitComp->first;
								ci.m_sideASurfaceId = sb->surface().id();
								fitComp->second.updateComponentType(*sb);
								m_componentInstances[ci.m_id] = ci;
								sb->m_componentInstanceId = ci.m_id;
							}
							else {
								ConvertError err;
								err.m_objectType = OT_Component;
								err.m_objectID = sb->m_id;
								err.m_errorText = "Component of element for space boundary not found in components list";
								errors.push_back(err);
							}
						}
						else {
							ConvertError err;
							err.m_objectType = OT_Instance;
							err.m_objectID = sb->m_id;
							err.m_errorText = "Element ID in space boundary not found in element list";
							errors.push_back(err);
						}
					}
				}
			}
		}
	}
}

void Instances::collectSubSurfaceComponentInstances(BuildingElementsCollector& elements, Database& database,
										 const Site& site, std::vector<ConvertError>& errors) {
	for(const auto& building : site.m_buildings) {
		for(const auto& storey : building->storeys()) {
			for(const auto& space : storey->spaces()) {
				for(const auto& sb : space->spaceBoundaries()) {
					if(!sb->isOpeningElement())
						continue;

					// don't go further if the space boundary is already assigned
					if(sb->m_componentInstanceId > -1)
						continue;


					if(sb->isMissing()) {
						ComponentInstance ci;
						ci.m_id = GUID_maker::instance().guid();
						ci.m_componentId = Database::m_missingComponentId;
						ci.m_sideASurfaceId = sb->surface().id();
						m_subSurfaceComponentInstances[ci.m_id] = ci;
						sb->m_componentInstanceId = ci.m_id;
					}
					else if(sb->isVirtual()) {
						ComponentInstance ci;
						ci.m_id = GUID_maker::instance().guid();
						ci.m_componentId = Database::m_virtualComponentId;
						ci.m_sideASurfaceId = sb->surface().id();
						m_subSurfaceComponentInstances[ci.m_id] = ci;
						sb->m_componentInstanceId = ci.m_id;
					}
					else {
						auto fitElem = std::find_if(
										   elements.m_openingElements.begin(),
										   elements.m_openingElements.end(),
										   [sb](const auto& elem) {return elem->m_id == sb->m_elementEntityId; });
						if(fitElem != elements.m_openingElements.end()) {
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
								ci.m_sideASurfaceId = sb->surface().id();
								m_subSurfaceComponentInstances[ci.m_id] = ci;
								sb->m_componentInstanceId = ci.m_id;
							}
							else {
								ConvertError err;
								err.m_objectType = OT_Instance;
								err.m_objectID = sb->m_id;
								err.m_errorText = "Element for opening space boundary not found in components list";
								errors.push_back(err);
							}
						}
						else {
							ConvertError err;
							err.m_objectType = OT_Instance;
							err.m_objectID = sb->m_id;
							err.m_errorText = "Element ID in opening space boundary not found in element list";
							errors.push_back(err);
						}
					}
				}
			}
		}
	}
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
