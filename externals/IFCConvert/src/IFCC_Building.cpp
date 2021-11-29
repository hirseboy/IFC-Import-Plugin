#include "IFCC_Building.h"

#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcBuilding.h>


#include "IFCC_Helper.h"

namespace IFCC {

Building::Building(int id) :
	EntityBase(id)
{}

bool Building::set(std::shared_ptr<IfcSpatialStructureElement> ifcElement) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcElement)))
		return false;

	const std::vector<weak_ptr<IfcRelAggregates> >& vec_decomposedBy = ifcElement->m_IsDecomposedBy_inverse;
	for(const auto& contEleme : vec_decomposedBy) {
		if( contEleme.expired() ) {
			continue;
		}
		shared_ptr<IfcRelAggregates> rel_aggregates( contEleme );
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
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
			std::string guid = converterX.to_bytes(opOrg->m_GlobalId->m_value);
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


} // namespace IFCC
