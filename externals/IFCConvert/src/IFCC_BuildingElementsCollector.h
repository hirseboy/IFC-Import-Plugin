#ifndef IFCC_BuildingElementsCollectorH
#define IFCC_BuildingElementsCollectorH

#include <vector>
#include <memory>
#include <string>

namespace IFCC {

class BuildingElement;

/*! Struct collects all types of building elements.*/
struct BuildingElementsCollector {
	/*! Primary building elements which are part of the construction and can contain openings.
		This can be: Wall, Roof, Slab
	*/
	std::vector<std::shared_ptr<BuildingElement>>		m_constructionElements;

	/*! Secondary building elements which are part of the construction and can contain openings.
		This can be: Beam, Column, Covering, Footing, CurtainWall
	*/
	std::vector<std::shared_ptr<BuildingElement>>		m_constructionSimilarElements;

	/*! All building elements which can be a opening.
		This can be: Window, Door
	*/
	std::vector<std::shared_ptr<BuildingElement>>		m_openingElements;

	/*! All other building elements.*/
	std::vector<std::shared_ptr<BuildingElement>>		m_otherElements;

	/*! Building elements without surfaces.*/
	std::vector<std::shared_ptr<BuildingElement>>		m_elementsWithoutSurfaces;

	void clear() {
		m_constructionElements.clear();
		m_constructionSimilarElements.clear();
		m_openingElements.clear();
		m_otherElements.clear();
		m_elementsWithoutSurfaces.clear();
	}

	std::vector<std::shared_ptr<BuildingElement>> allConstructionElements() const {
		std::vector<std::shared_ptr<BuildingElement>> constructionElements(m_constructionElements);
		constructionElements.insert(constructionElements.begin(), m_constructionSimilarElements.begin(),
									m_constructionSimilarElements.end());
		return constructionElements;
	}

	const std::shared_ptr<BuildingElement> fromGUID(const std::string& guid) const;

	std::shared_ptr<BuildingElement> fromID(int id) const;
};

} // namespace IFCC

#endif // IFCC_BuildingElementsCollectorH
