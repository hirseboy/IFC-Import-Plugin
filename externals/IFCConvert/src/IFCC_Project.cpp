#include "IFCC_Project.h"

#include <ifcpp/IFC4X3/include/IfcRelAggregates.h>
#include <ifcpp/IFC4X3/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4X3/include/IfcObjectDefinition.h>

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
#include "IFCC_Helper.h"
#include "IFCC_RepresentationHelper.h"

namespace IFCC {

Project::Project(int id) :
	EntityBase(id)
{

}

bool Project::set(std::shared_ptr<IFC4X3::IfcProject> ifcElement) {
	if(!EntityBase::set(dynamic_pointer_cast<IFC4X3::IfcRoot>(ifcElement)))
		return false;

	const std::vector<weak_ptr<IFC4X3::IfcRelAggregates> >& vec_decomposedBy = ifcElement->m_IsDecomposedBy_inverse;
	for(const auto& contEleme : vec_decomposedBy) {
		if( contEleme.expired() ) {
			continue;
		}
		shared_ptr<IFC4X3::IfcRelAggregates> rel_aggregates( contEleme );
		if( rel_aggregates ) {
			const std::vector<shared_ptr<IFC4X3::IfcObjectDefinition> >& vec_related_objects = rel_aggregates->m_RelatedObjects;
			for(const auto& contObj : vec_related_objects) {
				if( contObj ) {
					shared_ptr<IFC4X3::IfcBuilding> building = std::dynamic_pointer_cast<IFC4X3::IfcBuilding>(contObj);
					if(building != nullptr)
						m_buildingsOriginal.push_back(building);
				}
			}
		}
	}

	return true;
}

void Project::fetchBuildings(const std::map<std::string,shared_ptr<ProductShapeData>>& buildings) {
	for(const auto& shape : buildings) {
		for(const auto& opOrg : m_buildingsOriginal) {
			std::string guid = opOrg->m_GlobalId->m_value;
			if(shape.first == guid) {
				std::shared_ptr<Building> building = std::shared_ptr<Building>(new Building(GUID_maker::instance().guid()));
				if(building->set(opOrg)) {
					m_buildings.push_back(building);
				}
				break;
			}
		}
	}
}

const std::vector<std::shared_ptr<IFC4X3::IfcBuilding>>& Project::buildingsOriginal() const {
	return m_buildingsOriginal;
}

} // namespace IFCC
