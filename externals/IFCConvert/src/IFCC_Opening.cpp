#include "IFCC_Opening.h"

#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>

#include "IFCC_Helper.h"

namespace IFCC {

Opening::Opening(int id) :
	EntityBase(id)
{

}

bool Opening::set(std::shared_ptr<IfcFeatureElementSubtraction> ifcElement) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcElement)))
		return false;

	m_guid = guidFromObject(ifcElement.get());

	return true;
}

void Opening::update(std::shared_ptr<ProductShapeData> productShape) {
	transform(productShape);
	fetchGeometry(productShape);
}


void Opening::transform(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	m_transformMatrix = productShape->getTransform();
	if(m_transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(m_transformMatrix);
	}
}

void Opening::fetchGeometry(std::shared_ptr<ProductShapeData> productShape) {
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

//const meshVector_t& Opening::meshVector() const {
//	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
//		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
//	}

//	if(!m_meshSetClosedFinal.empty())
//		return m_meshSetClosedFinal;

//	return m_meshSetOpenFinal;
//}

const polyVector_t& Opening::polyVector() const {
	if(!m_polyvectClosedFinal.empty() && !m_polyvectOpenFinal.empty()) {
		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
	}

	if(!m_polyvectClosedFinal.empty())
		return m_polyvectClosedFinal;

	return m_polyvectOpenFinal;
}

const std::vector<Surface>& Opening::surfaces() const {
	return m_surfaces;
}

carve::mesh::Face<3>* Opening::face(FaceIndex findex) const {
	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
		return nullptr;
	}

	if(!m_meshSetClosedFinal.empty())
		return faceFromMeshset(m_meshSetClosedFinal, findex);

	return faceFromMeshset(m_meshSetOpenFinal, findex);
}

} // namespace IFCC
