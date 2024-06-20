#ifndef IFCC_ConvertOptionsH
#define IFCC_ConvertOptionsH

#include <QSet>

#include "IFCC_Types.h"

namespace IFCC {

class ConvertOptions
{
public:
	enum ConstructionMatching {
		CM_MatchEachConstruction,
		CM_MatchOnlyFirstConstruction,
		CM_MatchFirstNConstructions,
		CM_NoMatching
	};

	ConvertOptions();

	bool hasElementsForSpaceBoundaries(BuildingElementTypes type) const {
		return m_elementsForSpaceBoundaries.find(type) != m_elementsForSpaceBoundaries.end();
	}

	bool noSearchForOpenings(BuildingElementTypes type) const {
		return m_noSearchForOpeningsInTypes.contains(type);
	}

	void addElementsForOpenings(const QSet<BuildingElementTypes>& newTypes);

	std::set<BuildingElementTypes>	m_elementsForSpaceBoundaries;
	double							m_distanceFactor = 3.0;
	double							m_openingDistance = 0.5;
	ConstructionMatching			m_matchingType = CM_MatchEachConstruction;
	int								m_matchedConstructionNumbers = 2;
	double							m_minimumSurfaceArea = 0.001;
	double							m_distanceEps = 1e-4;
	bool							m_createMissingSite = true;

private:
	QSet<BuildingElementTypes>		m_noSearchForOpeningsInTypes;
	QSet<BuildingElementTypes>		m_noSearchForOpeningsInTypesFixed;
};

} // namespace IFCC

#endif // IFCC_ConvertOptionsH
