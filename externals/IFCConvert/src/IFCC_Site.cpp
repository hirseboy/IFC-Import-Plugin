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

	int repCount = productShape->m_vec_representations.size();
	std::shared_ptr<RepresentationData> currentRep;
	for(int repi = 0; repi<repCount; ++repi) {
		currentRep = productShape->m_vec_representations[repi];
		if(currentRep->m_representation_identifier == L"Body")
			break;
	}

	meshVector_t meshSetClosedFinal;
	meshVector_t meshSetOpenFinal;
	if(repCount > 0) {
		int itemDataCount = currentRep->m_vec_item_data.size();
		if(itemDataCount > 0) {
			meshSetClosedFinal = currentRep->m_vec_item_data.front()->m_meshsets;
			meshSetOpenFinal = currentRep->m_vec_item_data.front()->m_meshsets_open;
		}
	}

	// try to simplify meshes by merging all coplanar faces
	meshVector_t& currentMeshSets =  meshSetClosedFinal.empty() ? meshSetOpenFinal : meshSetClosedFinal;
	if(!currentMeshSets.empty()) {
		simplifyMesh(currentMeshSets, false);
	}

	polyVector_t polyvectClosedFinal;
	if(!meshSetClosedFinal.empty()) {
		int msCount = meshSetClosedFinal.size();
		for(int i=0; i<msCount; ++i) {
			polyvectClosedFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *meshSetClosedFinal[i];
			convert(currMeshSet, polyvectClosedFinal.back());
			// get surfaces
			for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
				for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
					if(currMeshSet.meshes[mi]->faces[fi] != nullptr)
						m_surfaces.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
				}
			}
		}
	}

	polyVector_t polyvectOpenFinal;
	if(!meshSetOpenFinal.empty()) {
		int msCount = meshSetOpenFinal.size();
		for(int i=0; i<msCount; ++i) {
			polyvectOpenFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *meshSetOpenFinal[i];
			convert(currMeshSet, polyvectOpenFinal.back());
			// get surfaces
			for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
				for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
					if(currMeshSet.meshes[mi]->faces[fi] != nullptr)
						m_surfaces.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
				}
			}
		}
	}
}

void Site::fetchBuildings(const std::map<std::string,shared_ptr<ProductShapeData>>& buildings) {
	for(const auto& shape : buildings) {
		for(const auto& opOrg : m_buildingsOriginal) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
			std::string guid = converterX.to_bytes(opOrg->m_GlobalId->m_value);
			if(shape.first == guid) {
				Building building(GUID_maker::instance().guid());
				if(building.set(opOrg)) {
					m_buildings.push_back(building);
				}
				break;
			}
		}
	}
}

TiXmlElement * Site::writeXML(TiXmlElement * parent) const {
	if(m_buildings.empty())
		return nullptr;

	TiXmlElement * e = new TiXmlElement("Buildings");
	parent->LinkEndChild(e);

	for( const Building& building : m_buildings) {
		building.writeXML(e);
	}
	return e;
}

const std::vector<Surface>& Site::surfaces() const {
	return m_surfaces;
}

} // namespace IFCC
