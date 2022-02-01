#include "IFCC_Building.h"

#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcBuilding.h>


#include <VICUS_Project.h>

#include "IFCC_Helper.h"

namespace IFCC {

Building::Building(int id) :
	EntityBase(id)
{}

bool Building::set(std::shared_ptr<IfcSpatialStructureElement> ifcElement) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcElement)))
		return false;

	const std::vector<weak_ptr<IfcRelAggregates> >& vec_decomposedBy = ifcElement->m_IsDecomposedBy_inverse;
	for(const auto& contentElement : vec_decomposedBy) {
		// weak pointer contentElement valid?
		if( contentElement.expired() ) {
			continue;
		}
		shared_ptr<IfcRelAggregates> rel_aggregates( contentElement );
		if( rel_aggregates ) {
			const std::vector<shared_ptr<IfcObjectDefinition> >& vec_related_objects = rel_aggregates->m_RelatedObjects;
			for(const auto& contObj : vec_related_objects) {
				if( contObj ) {
					shared_ptr<IfcBuildingStorey> storey = std::dynamic_pointer_cast<IfcBuildingStorey>(contObj);
					if(storey != nullptr)
						m_storeysOriginal.push_back(storey);
				}
			}
		}
	}

	return true;
}

void Building::fetchStoreys(const std::map<std::string,shared_ptr<ProductShapeData>>& storeys) {
	for(const auto& shape : storeys) {
		for(const auto& opOrg : m_storeysOriginal) {
			std::string guid = guidFromObject(opOrg.get());
			if(shape.first == guid) {
				BuildingStorey storey(GUID_maker::instance().guid());
				if(storey.set(opOrg)) {
					m_storeys.push_back(storey);
				}
				break;
			}
		}
	}
}

void Building::updateStoreys(const objectShapeTypeVector_t& elementShapes,
							 const objectShapeGUIDMap_t& spaceShapes,
							 shared_ptr<UnitConverter>& unit_converter,
							 const std::vector<BuildingElement>& constructionElemnts,
							 const std::vector<BuildingElement>& openingElements,
							 const std::vector<Opening>& openings,
							 bool useSpaceBoundaries) {

	for(auto& storey : m_storeys) {
		storey.fetchSpaces(spaceShapes, unit_converter);
		storey.updateSpaces(elementShapes, unit_converter, constructionElemnts, openingElements, openings, useSpaceBoundaries);
	}
}


TiXmlElement * Building::writeXML(TiXmlElement * parent) const {
	if (m_id == -1)
		return nullptr;

	TiXmlElement * e = new TiXmlElement("Building");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
//	e->SetAttribute("visible", IBK::val2string<bool>(true));

	if(!m_storeys.empty()) {
		TiXmlElement * child = new TiXmlElement("BuildingLevels");
		e->LinkEndChild(child);

		for( const BuildingStorey& storey : m_storeys) {
			storey.writeXML(child);
		}
	}
	return e;
}

VICUS::Building Building::getVicusObject(std::map<int,int>& idMap, VICUS::Project* project) const {
	VICUS::Building res;
	int newId = project->nextUnusedID();

	res.m_displayName = QString::fromUtf8(m_name.c_str());
	res.m_id = newId;
	idMap[m_id] = newId;
	for(const auto& storey : m_storeys) {
		res.m_buildingLevels.emplace_back(storey.getVicusObject(idMap, project));
	}

	return res;
}

} // namespace IFCC
