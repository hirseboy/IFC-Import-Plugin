#include "IFCC_BuildingElementsCollector.h"

#include "IFCC_BuildingElement.h"

namespace IFCC {

const std::shared_ptr<BuildingElement> BuildingElementsCollector::fromGUID(const std::string& guid) const {
	for(auto elem : m_constructionElements) {
		if(elem->m_guid == guid)
			return elem;
	}
	for(auto elem : m_constructionSimilarElements) {
		if(elem->m_guid == guid)
			return elem;
	}
	for(auto elem : m_openingElements) {
		if(elem->m_guid == guid)
			return elem;
	}
	for(auto elem : m_otherElements) {
		if(elem->m_guid == guid)
			return elem;
	}
	for(auto elem : m_elementsWithoutSurfaces) {
		if(elem->m_guid == guid)
			return elem;
	}

	return {};	  // std::shared_ptr<BuildingElement>(nullptr);
}

std::shared_ptr<BuildingElement> BuildingElementsCollector::fromID(int id) const {
	for(auto elem : m_constructionElements) {
		if(elem->m_id == id)
			return elem;
	}
	for(auto elem : m_constructionSimilarElements) {
		if(elem->m_id == id)
			return elem;
	}
	for(auto elem : m_openingElements) {
		if(elem->m_id == id)
			return elem;
	}
	for(auto elem : m_otherElements) {
		if(elem->m_id == id)
			return elem;
	}
	for(auto elem : m_elementsWithoutSurfaces) {
		if(elem->m_id == id)
			return elem;
	}

	return {};	  // std::shared_ptr<BuildingElement>(nullptr);

}


} // namespace IFCC
