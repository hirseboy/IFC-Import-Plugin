#include "IFCC_BuildingStorey.h"

#include <ifcpp/IFC4/include/IfcRelDefinesByProperties.h>
#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>

#include "IFCC_Helper.h"

namespace IFCC {

BuildingStorey::BuildingStorey(int id) :
	EntityBase(id)
{}

bool BuildingStorey::set(std::shared_ptr<IfcSpatialStructureElement> ifcElement) {
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
					shared_ptr<IfcSpace> space = std::dynamic_pointer_cast<IfcSpace>(contObj);
					if(space != nullptr)
						m_spacesOriginal.push_back(space);
				}
			}
		}
	}

	return true;
}

void BuildingStorey::fetchSpaces(const std::map<std::string,shared_ptr<ProductShapeData>>& shapes,
								 shared_ptr<UnitConverter>& unit_converter) {
	for(const auto& shape : shapes) {
		for(const auto& opOrg : m_spacesOriginal) {
			if(shape.first == guidFromObject(opOrg.get())) {
				Space space(GUID_maker::instance().guid());
				if(space.set(opOrg)) {
					m_spaces.push_back(space);
					m_spaces.back().update(shape.second);
				}
				break;
			}
		}
	}
}

TiXmlElement * BuildingStorey::writeXML(TiXmlElement * parent) const {
	if (m_id == -1)
		return nullptr;

	TiXmlElement * e = new TiXmlElement("BuildingLevel");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("displayName", m_name);
//	e->SetAttribute("visible", IBK::val2string<bool>(true));
	TiXmlElement::appendSingleAttributeElement(e, "Elevation", nullptr, std::string(), IBK::val2string<double>(0));
	TiXmlElement::appendSingleAttributeElement(e, "Height", nullptr, std::string(), IBK::val2string<double>(3));

	if(!m_spaces.empty()) {
		TiXmlElement * child = new TiXmlElement("Rooms");
		e->LinkEndChild(child);

		for( const Space& space : m_spaces) {
			space.writeXML(child);
		}
	}
	return e;
}

} // namespace IFCC