#include "IFCC_Site.h"

#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcObjectDefinition.h>

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>

#include "IFCC_Helper.h"

namespace IFCC {

Site::Site(int id) :
	EntityBase(id)
{

}

bool Site::set(std::shared_ptr<IfcSpatialStructureElement> ifcElement) {
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
					shared_ptr<IfcBuilding> building = std::dynamic_pointer_cast<IfcBuilding>(contObj);
					if(building != nullptr)
						m_buildingsOriginal.push_back(building);
				}
			}
		}
	}

	return true;
}

bool Site::set(std::shared_ptr<IfcSpatialStructureElement> ifcElement, std::shared_ptr<ProductShapeData> productShape,
			   const std::map<std::string,shared_ptr<ProductShapeData>>& buildings) {
	if(!set(ifcElement))
		return false;

	transform(productShape);
	fetchGeometry(productShape);
	fetchBuildings(buildings);
	return true;
}


void Site::transform(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	carve::math::Matrix transformMatrix = productShape->getTransform();
	if(transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(transformMatrix);
	}
}

void Site::fetchGeometry(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	surfacesFromRepresentation(productShape, m_surfaces);
}

void Site::fetchBuildings(const std::map<std::string,shared_ptr<ProductShapeData>>& buildings) {
	for(const auto& shape : buildings) {
		for(const auto& opOrg : m_buildingsOriginal) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
			std::string guid = converterX.to_bytes(opOrg->m_GlobalId->m_value);
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

std::vector<std::shared_ptr<SpaceBoundary>> Site::allSpaceBoundaries() const {
	std::vector<std::shared_ptr<SpaceBoundary>> res;
	for(const std::shared_ptr<Building>& building : m_buildings) {
		const std::vector<std::shared_ptr<BuildingStorey>>& storeys = building->storeys();
		for(const std::shared_ptr<BuildingStorey>& storey : storeys) {
			const std::vector<std::shared_ptr<Space>>& spaces = storey->spaces();
			for(const std::shared_ptr<Space>& space : spaces) {
				const std::vector<std::shared_ptr<SpaceBoundary>>& sbs = space->spaceBoundaries();
				res.insert(res.begin(), sbs.begin(), sbs.end());
			}
		}
	}
	return res;
}

std::vector<std::shared_ptr<Space>> Site::allSpaces() const {
	std::vector<std::shared_ptr<Space>> res;
	for(const std::shared_ptr<Building>& building : m_buildings) {
		const std::vector<std::shared_ptr<BuildingStorey>>& storeys = building->storeys();
		for(const std::shared_ptr<BuildingStorey>& storey : storeys) {
			const std::vector<std::shared_ptr<Space>>& spaces = storey->spaces();
			res.insert(res.begin(), spaces.begin(), spaces.end());
		}
	}
	return res;
}

///*! Return a vector of all space connection in project.*/
//std::vector<Space::SurfaceConnectionVectors> Site::allSurfaceConnectionVectors() const {
//	std::vector<Space::SurfaceConnectionVectors> res;
//	for(const std::shared_ptr<Building>& building : m_buildings) {
//		const std::vector<std::shared_ptr<BuildingStorey>>& storeys = building->storeys();
//		for(const std::shared_ptr<BuildingStorey>& storey : storeys) {
//			const std::vector<std::shared_ptr<Space>>& spaces = storey->spaces();
//			for(const std::shared_ptr<Space>& space : spaces) {
//				res.push_back(space->surfaceConnectionVectors());
//			}
//		}
//	}
//	return res;
//}


TiXmlElement * Site::writeXML(TiXmlElement * parent, bool flipPolygons) const {
	if(m_buildings.empty())
		return nullptr;

	TiXmlElement * e = new TiXmlElement("Buildings");
	parent->LinkEndChild(e);

	for( const std::shared_ptr<Building>& building : m_buildings) {
		building->writeXML(e, flipPolygons);
	}
	return e;
}

const std::vector<Surface>& Site::surfaces() const {
	return m_surfaces;
}

} // namespace IFCC
