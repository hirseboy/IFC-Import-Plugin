#include "IFCC_Instances.h"


#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
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
//			if(comInst.second.m_sideASurfaceId > 0)
				comInst.second.writeXML(child);
		}
	}

	if(!m_subSurfaceComponentInstances.empty()) {
		TiXmlElement * child = new TiXmlElement("SubSurfaceComponentInstances");
		parent->LinkEndChild(child);

		for(const auto& comInst : m_subSurfaceComponentInstances) {
//			if(comInst.second.m_sideASurfaceId > 0)
				comInst.second.writeXML(child);
		}
	}

	return parent;
}

void Instances::collectComponentInstances(BuildingElementsCollector& elements, Database& database, const Site& site,
										 std::vector<ConvertError>& errors, const ConvertOptions& convertOptions) {
	collectNormalComponentInstances(elements, database, site, errors, convertOptions);
	collectSubSurfaceComponentInstances(elements, database, site, errors, convertOptions);
}

void Instances::collectNormalComponentInstances(BuildingElementsCollector& elements, Database& database,
										 const Site& site, std::vector<ConvertError>& errors, const ConvertOptions& convertOptions) {
	std::vector<std::shared_ptr<BuildingElement>> constructionElements = elements.allConstructionElements();
	for(const auto& building : site.m_buildings) {
		for(const auto& storey : building->storeys()) {
			for(const auto& space : storey->spaces()) {
				for(const auto& sb : space->spaceBoundaries()) {
					// first loop is only for normal component instances
					if(!sb->isConstructionElement())
						continue;

					// it makes no sense to add a space boundary with a non valid surface
					if(!sb->surface().check(convertOptions.m_polygonEps))
						continue;

					// don't go further if the space boundary is already assigned
					if(sb->m_componentInstanceId > -1)
						continue;

					if(sb->isMissing()) {
						ComponentInstance ci(GUID_maker::instance().guid(), Database::m_missingComponentId, sb->surface().id());
						if(sb->surface().id() >= 0) {
							m_componentInstances[ci.id()] = ci;
							sb->m_componentInstanceId = ci.id();
						}
						else {
							errors.push_back(ConvertError{OT_Instance, sb->m_id, "Creating component instance - space boundary with non valid surface Id found"});
						}
					}
					else if(sb->isVirtual()) {
						ComponentInstance ci(GUID_maker::instance().guid(), Database::m_virtualComponentId, sb->surface().id());
						if(sb->surface().id() >= 0) {
							m_componentInstances[ci.id()] = ci;
							sb->m_componentInstanceId = ci.id();
						}
						else {
							errors.push_back(ConvertError{OT_Instance, sb->m_id, "Creating component instance - space boundary with non valid surface Id found"});
						}
					}
					else {
						auto fitElem = std::find_if(
										   constructionElements.begin(),
										   constructionElements.end(),
										   [sb](const auto& elem) -> bool {return elem->m_id == sb->m_elementEntityId; });
						if(fitElem != constructionElements.end()) {
							const std::shared_ptr<BuildingElement>& elem = *fitElem;
							auto fitComp = std::find_if(
											   database.m_components.begin(),
											   database.m_components.end(),
											   [elem](const auto& comp) -> bool {return comp.second.m_guid == elem->m_guid; });
							if(fitComp != database.m_components.end()) {
								ComponentInstance ci(GUID_maker::instance().guid(), fitComp->first, sb->surface().id());
								fitComp->second.updateComponentType(*sb);
								if(sb->surface().id() >= 0 ) {
									m_componentInstances[ci.id()] = ci;
									sb->m_componentInstanceId = ci.id();
								}
								else {
									errors.push_back(ConvertError{OT_Instance, sb->m_id, "Creating component instance - space boundary with non valid surface Id found"});
								}
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
										 const Site& site, std::vector<ConvertError>& errors, const ConvertOptions& convertOptions) {
	for(const auto& building : site.m_buildings) {
		for(const auto& storey : building->storeys()) {
			for(const auto& space : storey->spaces()) {
				for(const auto& subsb : space->spaceBoundaries()) {
					if(!subsb->isConstructionElement())
						continue;

					if(!subsb->surface().check(convertOptions.m_polygonEps))
						continue;

					// go through all subsurfaces of the current space boundary
					for(const auto& subsb : subsb->containedOpeningSpaceBoundaries()) {

						// don't go further if the space boundary is already assigned
						if(subsb->m_componentInstanceId > -1)
							continue;

						if(subsb->isMissing()) {
							ComponentInstance ci(GUID_maker::instance().guid(), Database::m_missingComponentId, subsb->surface().id());
							ci.setSubSurface(true);
							if(subsb->surface().id() >= 0) {
								m_subSurfaceComponentInstances[ci.id()] = ci;
								subsb->m_componentInstanceId = ci.id();
							}
							else {
								errors.push_back(ConvertError{OT_Instance, subsb->m_id, "Creating subsurface component instance - space boundary with non valid surface Id found"});
							}
						}
						else if(subsb->isVirtual()) {
							// ComponentInstance ci(GUID_maker::instance().guid(), Database::m_virtualComponentId, subsb->surface().id());
							// ci.setSubSurface(true);
							// if(subsb->surface().id() >= 0) {
							// 	m_subSurfaceComponentInstances[ci.id()] = ci;
							// 	subsb->m_componentInstanceId = ci.id();
							// }
							// else {
							// 	errors.push_back(ConvertError{OT_Instance, subsb->m_id, "Creating subsurface component instance - space boundary with non valid surface Id found"});
							// }
						}
						else {
							auto fitElem = std::find_if(
										elements.m_openingElements.begin(),
										elements.m_openingElements.end(),
										[subsb](const auto& elem) -> bool {return elem->m_id == subsb->m_elementEntityId; });
							if(fitElem != elements.m_openingElements.end()) {
								const std::shared_ptr<BuildingElement>& elem = *fitElem;
								auto fitComp = std::find_if(
											database.m_subSurfaceComponents.begin(),
											database.m_subSurfaceComponents.end(),
											[elem](const auto& comp) -> bool {return comp.second.guid() == elem->m_guid; });
								if(fitComp != database.m_subSurfaceComponents.end()) {
									ComponentInstance ci(GUID_maker::instance().guid(), fitComp->first, subsb->surface().id());
									ci.setSubSurface(true);
									if(subsb->surface().id() >= 0) {
										m_subSurfaceComponentInstances[ci.id()] = ci;
										subsb->m_componentInstanceId = ci.id();
									}
									else {
										errors.push_back(ConvertError{OT_Instance, subsb->m_id, "Creating subsurface component instance - space boundary with non valid surface Id found"});
									}
								}
								else {
									ConvertError err;
									err.m_objectType = OT_Instance;
									err.m_objectID = subsb->m_id;
									err.m_errorText = "Element for opening space boundary not found in components list";
									errors.push_back(err);
								}
							}
							else {
								ConvertError err;
								err.m_objectType = OT_Instance;
								err.m_objectID = subsb->m_id;
								err.m_errorText = "Element ID in opening space boundary not found in element list";
								errors.push_back(err);
							}
						}
					}
				}
			}
		}
	}
}

static bool hasSurfaceId(SpaceBoundary* sb, int id) {
	if(sb == nullptr)
		return false;
	if(sb->surface().id() == id)
		return true;
	for(const auto& ss : sb->surface().subSurfaces()) {
		if(ss.id() == id)
			return true;
	}
	return false;
}

static bool hasSubSurfaceId(SpaceBoundary* sb, int id) {
	if(sb == nullptr)
		return false;
	for(const auto& ss : sb->surface().subSurfaces()) {
		if(ss.id() == id)
			return true;
	}
	return false;
}

static bool hasSurfaceId(const std::vector<std::shared_ptr<SpaceBoundary>>& sbs, int id) {
	for(const auto& sb : sbs) {
		if(hasSurfaceId(sb.get(), id))
			return true;
	}
	return false;
}

static bool hasSubSurfaceId(const std::vector<std::shared_ptr<SpaceBoundary>>& sbs, int id) {
	for(const auto& sb : sbs) {
		if(hasSubSurfaceId(sb.get(), id))
			return true;
	}
	return false;
}

std::vector<int> Instances::checkForWrongSurfaceIds(const Site& site) {
	std::vector<int> res;
	std::vector<std::shared_ptr<SpaceBoundary>> allSBs = site.allSpaceBoundaries();
	for(const auto& ci : m_componentInstances) {
		if(!hasSurfaceId(allSBs, ci.second.sideASurfaceId()))
			res.push_back(ci.second.id());
	}
	for(const auto& ci : m_subSurfaceComponentInstances) {
		if(!hasSubSurfaceId(allSBs, ci.second.sideASurfaceId()))
			res.push_back(ci.second.id());
	}
	return res;
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
