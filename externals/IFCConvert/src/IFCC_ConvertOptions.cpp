#include "IFCC_ConvertOptions.h"

#include "IFCC_Helper.h"

namespace IFCC {

ConvertOptions::ConvertOptions() {
	m_noSearchForOpeningsInTypesFixed = constructionSimilarTypes();
	m_noSearchForOpeningsInTypesFixed += openingTypes();
	m_noSearchForOpeningsInTypesFixed << BET_Chimney << BET_RampFlight << BET_ShadingDevice;
	m_noSearchForOpeningsInTypesFixed << BET_Member << BET_Pile << BET_Plate << BET_Railing << BET_Ramp;
	m_noSearchForOpeningsInTypesFixed << BET_Stair << BET_StairFlight << BET_CivilElement << BET_DistributionElement;
	m_noSearchForOpeningsInTypesFixed << BET_ElementAssembly << BET_ElementComponent << BET_FeatureElement << BET_FurnishingElement;
	m_noSearchForOpeningsInTypesFixed << BET_GeographicalElement << BET_TransportElement << BET_VirtualElement;

	m_noSearchForOpeningsInTypes = m_noSearchForOpeningsInTypesFixed;
}

void ConvertOptions::addElementsForOpenings(const QSet<BuildingElementTypes> &newTypes) {
	m_noSearchForOpeningsInTypes = m_noSearchForOpeningsInTypesFixed;
	m_noSearchForOpeningsInTypes += newTypes;
}

} // namespace IFCC
