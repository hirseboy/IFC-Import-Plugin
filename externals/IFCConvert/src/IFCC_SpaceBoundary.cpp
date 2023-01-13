#include "IFCC_SpaceBoundary.h"

#include <ifcpp/IFC4/include/IfcConnectionCurveGeometry.h>
#include <ifcpp/IFC4/include/IfcConnectionPointEccentricity.h>
#include <ifcpp/IFC4/include/IfcConnectionPointGeometry.h>
#include <ifcpp/IFC4/include/IfcConnectionSurfaceGeometry.h>
#include <ifcpp/IFC4/include/IfcConnectionVolumeGeometry.h>
#include <ifcpp/IFC4/include/IfcSurface.h>
#include <ifcpp/IFC4/include/IfcSpace.h>
#include <ifcpp/IFC4/include/IfcExternalSpatialElement.h>
#include <ifcpp/IFC4/include/IfcFaceBasedSurfaceModel.h>
#include <ifcpp/IFC4/include/IfcFaceSurface.h>
#include <ifcpp/IFC4/include/IfcRelSpaceBoundary1stLevel.h>
#include <ifcpp/IFC4/include/IfcRelSpaceBoundary2ndLevel.h>


#include "IFCC_Helper.h"
#include "IFCC_Types.h"
#include "IFCC_RepresentationConverter.h"
#include "IFCC_Database.h"
#include "IFCC_Space.h"


namespace IFCC {

SpaceBoundary::SpaceBoundary(int id) :
	EntityBase(id),
	m_elementEntityId(-1),
	m_openingId(-1),
	m_componentInstanceId(-1),
	m_typeRelatedElement(BET_All),
	m_physicalOrVirtual(IfcPhysicalOrVirtualEnum::ENUM_NOTDEFINED),
	m_internalOrExternal(IfcInternalOrExternalEnum::ENUM_NOTDEFINED),
	m_type(CT_Unknown),
	m_spaceBoundaryType(SBT_Unknown),
	m_levelType(SBLT_NoLevel)
{

}

bool SpaceBoundary::setFromIFC(std::shared_ptr<IfcRelSpaceBoundary> ifcSpaceBoundary) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcSpaceBoundary)))
		return false;

	if(ifcSpaceBoundary->m_RelatedBuildingElement != nullptr) {
		m_guidRelatedElement = guidFromObject(ifcSpaceBoundary->m_RelatedBuildingElement.get());
		m_nameRelatedElement = label2s(ifcSpaceBoundary->m_RelatedBuildingElement->m_Name);
	}

	if(ifcSpaceBoundary->m_RelatingSpace != nullptr) {
		shared_ptr<IfcSpace> space = dynamic_pointer_cast<IfcSpace>(ifcSpaceBoundary->m_RelatingSpace);
		if(space) {
			m_guidRelatedSpace = guidFromObject(space.get());
			m_nameRelatedSpace = label2s(space->m_Name);
		}
		else {
			shared_ptr<IfcExternalSpatialElement> externalSpace = dynamic_pointer_cast<IfcExternalSpatialElement>(ifcSpaceBoundary->m_RelatingSpace);
			if(externalSpace) {
				m_guidRelatedSpace = guidFromObject(externalSpace.get());
				m_nameRelatedSpace = label2s(externalSpace->m_Name);
			}
		}
	}

	if(ifcSpaceBoundary->m_PhysicalOrVirtualBoundary != nullptr) {
		m_physicalOrVirtual = ifcSpaceBoundary->m_PhysicalOrVirtualBoundary->m_enum;
//		if(m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL)
//			return false;
	}
	if(ifcSpaceBoundary->m_InternalOrExternalBoundary != nullptr) {
		m_internalOrExternal = ifcSpaceBoundary->m_InternalOrExternalBoundary->m_enum;
	}

	if(m_name == "2ndLevel")
		m_levelType = SBLT_2ndLevel;
	if(m_name == "1stLevel")
		m_levelType = SBLT_1stLevel;
	if(m_description == "2a")
		m_spaceBoundaryType = SBT_A;
	if(m_description == "2b")
		m_spaceBoundaryType = SBT_B;

	shared_ptr<IfcRelSpaceBoundary1stLevel> sb1stLevel = std::dynamic_pointer_cast<IfcRelSpaceBoundary1stLevel>(ifcSpaceBoundary);
	if(sb1stLevel) {
		m_levelType = SBLT_1stLevel;
		if(sb1stLevel->m_ParentBoundary != nullptr) {
			m_spaceBoundaryType = SBT_Inner;
		}
	}

	shared_ptr<IfcRelSpaceBoundary2ndLevel> sb2ndLevel = std::dynamic_pointer_cast<IfcRelSpaceBoundary2ndLevel>(ifcSpaceBoundary);
	if(sb2ndLevel) {
		m_levelType = SBLT_2ndLevel;
		if(sb2ndLevel->m_CorrespondingBoundary != nullptr) {
			m_guidCorrespondingBoundary = guidFromObject(sb2ndLevel->m_CorrespondingBoundary.get());
			if(m_spaceBoundaryType != SBT_Inner)
				m_spaceBoundaryType = SBT_A;
		}
		else {
			if(m_spaceBoundaryType != SBT_Inner)
				m_spaceBoundaryType = SBT_B;
		}
	}

	m_connectionGeometry = ifcSpaceBoundary->m_ConnectionGeometry;
	return true;
}

bool SpaceBoundary::setFromBuildingElement(const std::string& name, const std::shared_ptr<BuildingElement>& elem,
										   const Space& space) {
	m_name = name;
	setRelatingElementType(elem->type());
	m_physicalOrVirtual = IfcPhysicalOrVirtualEnum::ENUM_PHYSICAL;
	m_guidRelatedElement = elem->m_guid;
	m_nameRelatedElement = elem->m_name;
	m_guidRelatedSpace = space.m_guid;
	m_nameRelatedSpace = space.m_name;
	return true;
}

void SpaceBoundary::setFromSpaceBoundary(const SpaceBoundary& sb, size_t surfaceIndex) {
	m_name = sb.m_name;
	m_description = sb.m_description;
	m_elementEntityId = sb.m_elementEntityId;
	m_guidRelatedElement = sb.m_guidRelatedElement;
	m_guidRelatedSpace = sb.m_guidRelatedSpace;
	m_guidCorrespondingBoundary = sb.m_guidCorrespondingBoundary;
	m_nameRelatedElement = sb.m_nameRelatedElement;
	m_nameRelatedSpace = sb.m_nameRelatedSpace;
	m_typeRelatedElement = sb.m_typeRelatedElement;
	m_physicalOrVirtual = sb.m_physicalOrVirtual;
	m_internalOrExternal = sb.m_internalOrExternal;
	m_type = sb.m_type;
	m_spaceBoundaryType = sb.m_spaceBoundaryType;
	m_levelType = sb.m_levelType;
	IBK_ASSERT(surfaceIndex > 0);
	IBK_ASSERT(surfaceIndex < sb.m_surfaces.size());
	m_surface = sb.m_surfaces[surfaceIndex];
}

void SpaceBoundary::setForMissingElement(const std::string& name, const Space& space) {
	m_name = name;
	m_typeRelatedElement = BET_None;
	m_type = CT_Others;
	m_spaceBoundaryType = SBT_Unknown;
	m_levelType = SBLT_NoLevel;
	m_elementEntityId = -1;
	m_physicalOrVirtual = IfcPhysicalOrVirtualEnum::ENUM_PHYSICAL;
	m_guidRelatedElement.clear();
	m_nameRelatedElement.clear();
	m_guidRelatedSpace = space.m_guid;
	m_nameRelatedSpace = space.m_name;
}

void SpaceBoundary::setForVirtualElement(const std::string& name, const Space& space) {
	m_name = name;
	m_typeRelatedElement = BET_None;
	m_type = CT_Others;
	m_spaceBoundaryType = SBT_Unknown;
	m_levelType = SBLT_NoLevel;
	m_elementEntityId = -1;
	m_physicalOrVirtual = IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL;
	m_guidRelatedElement.clear();
	m_nameRelatedElement.clear();
	m_guidRelatedSpace = space.m_guid;
	m_nameRelatedSpace = space.m_name;
}

void SpaceBoundary::setRelatingElementType(BuildingElementTypes type) {
	m_typeRelatedElement = type;
	if(isConstructionType(type))
		m_type = CT_ConstructionElement;
	else if(isConstructionSimilarType(type))
		m_type = CT_ConstructionElement;
	else if(isOpeningType(type))
		m_type = CT_OpeningElement;
	else m_type = CT_Others;
}

bool SpaceBoundary::fetchGeometryFromIFC(shared_ptr<UnitConverter>& unit_converter,
										 const carve::math::Matrix& spaceTransformation, std::vector<ConvertError>& errors) {

	polyVector_t polylines = polylinesFromConnectionGeometry(m_connectionGeometry, unit_converter, spaceTransformation,
															 m_id, errors);
	if(polylines.empty())
		return false;

	createSurfaceVect(polylines);
	return true;
}

bool SpaceBoundary::fetchGeometryFromBuildingElement(const Surface& surface) {
	m_surfaces.clear();
	if(surface.isValid()) {
		std::string name = m_nameRelatedElement;
		if(name.empty())
			name = std::to_string(m_id);
		m_surface = surface;
		m_surface.set(GUID_maker::instance().guid(), m_elementEntityId, name,
							  m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL);
		m_surface.setSurfaceType(m_internalOrExternal);
		return true;
	}
	return false;
}

std::vector<std::shared_ptr<SpaceBoundary>> SpaceBoundary::splitBySurfaces() {
	std::vector<std::shared_ptr<SpaceBoundary>> res;
	if(m_surfaces.empty())
		return res;

	for(size_t i=1; i<m_surfaces.size(); ++i) {
		res.emplace_back(std::shared_ptr<SpaceBoundary>(new SpaceBoundary(GUID_maker::instance().guid())));
		res.back()->setFromSpaceBoundary(*this, i);
	}
	m_surface = m_surfaces.front();
	m_surfaces.clear();
	return res;
}

void SpaceBoundary::createSurfaceVect(const polyVector_t& polylines) {
	m_surfaces.clear();
	std::string name = m_nameRelatedElement;
	if(name.empty())
		name = std::to_string(m_id);
	for(const auto& polyvect3 : polylines) {
		for(const auto& polyvect2 : polyvect3) {
			for(const auto& polyvect1 : polyvect2) {
				if(!polyvect1.empty()) {
					m_surfaces.emplace_back(Surface(polyvect1));
					m_surfaces.back().set(GUID_maker::instance().guid(), m_elementEntityId, name,
										  m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL);
					m_surfaces.back().setSurfaceType(m_internalOrExternal);
				}
			}
		}
	}
}

const std::vector<std::shared_ptr<SpaceBoundary> >& SpaceBoundary::containedOpeningSpaceBoundaries() const {
	return m_containedOpeningSpaceBoundaries;
}

void SpaceBoundary::addContainedOpeningSpaceBoundaries(const std::shared_ptr<SpaceBoundary>& newContainedOpeningSpaceBoundaries) {
	m_containedOpeningSpaceBoundaries.push_back(newContainedOpeningSpaceBoundaries);
}

} // namespace IFCC
