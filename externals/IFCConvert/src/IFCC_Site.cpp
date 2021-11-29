#include "IFCC_Site.h"

#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcObjectDefinition.h>

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

	m_transformMatrix = productShape->getTransform();
	if(m_transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(m_transformMatrix);
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
	if(repCount > 0) {
		int itemDataCount = currentRep->m_vec_item_data.size();
		if(itemDataCount > 0) {
			m_meshSetClosedFinal = currentRep->m_vec_item_data.front()->m_meshsets;
			m_meshSetOpenFinal = currentRep->m_vec_item_data.front()->m_meshsets_open;
		}
	}

	// try to simplify meshes by merging all coplanar faces
	meshVector_t& currentMeshSets =  m_meshSetClosedFinal.empty() ? m_meshSetOpenFinal : m_meshSetClosedFinal;
	if(!currentMeshSets.empty()) {
		simplifyMesh(currentMeshSets, false);
	}

	if(!m_meshSetClosedFinal.empty()) {
		int msCount = m_meshSetClosedFinal.size();
		for(int i=0; i<msCount; ++i) {
			m_polyvectClosedFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *m_meshSetClosedFinal[i];
			convert(currMeshSet, m_polyvectClosedFinal.back());
			// get surfaces
			for(size_t mi=0; mi<currMeshSet.meshes.size(); ++mi) {
				for(size_t fi =0; fi<currMeshSet.meshes[mi]->faces.size(); ++fi) {
					if(currMeshSet.meshes[mi]->faces[fi] != nullptr)
						m_surfaces.emplace_back(Surface(currMeshSet.meshes[mi]->faces[fi]));
				}
			}
		}
	}
	if(!m_meshSetOpenFinal.empty()) {
		int msCount = m_meshSetOpenFinal.size();
		for(int i=0; i<msCount; ++i) {
			m_polyvectOpenFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
			const carve::mesh::MeshSet<3>& currMeshSet = *m_meshSetOpenFinal[i];
			convert(currMeshSet, m_polyvectOpenFinal.back());
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

const meshVector_t& Site::meshVector() const {
	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
	}

	if(!m_meshSetClosedFinal.empty())
		return m_meshSetClosedFinal;

	return m_meshSetOpenFinal;
}

const polyVector_t& Site::polyVector() const {
	if(!m_polyvectClosedFinal.empty() && !m_polyvectOpenFinal.empty()) {
		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
	}

	if(!m_polyvectClosedFinal.empty())
		return m_polyvectClosedFinal;

	return m_polyvectOpenFinal;
}

const std::vector<Surface>& Site::surfaces() const {
	return m_surfaces;
}

carve::mesh::Face<3>* Site::face(FaceIndex findex) const {
	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
		return nullptr;
	}

	if(!m_meshSetClosedFinal.empty())
		return faceFromMeshset(m_meshSetClosedFinal, findex);

	return faceFromMeshset(m_meshSetOpenFinal, findex);
}

} // namespace IFCC
