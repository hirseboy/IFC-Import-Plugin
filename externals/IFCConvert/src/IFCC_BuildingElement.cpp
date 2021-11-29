#include "IFCC_BuildingElement.h"

#include <ifcpp/IFC4/include/IfcRelVoidsElement.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcObjectTypeEnum.h>
#include <ifcpp/IFC4/include/IfcMaterialLayerSetUsage.h>
#include <ifcpp/IFC4/include/IfcMaterialLayerSet.h>
#include <ifcpp/IFC4/include/IfcMaterialLayer.h>
#include <ifcpp/IFC4/include/IfcMaterial.h>
#include <ifcpp/IFC4/include/IfcNonNegativeLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcPositiveLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcWindowStyle.h>
#include <ifcpp/IFC4/include/IfcWindowStyleOperationEnum.h>
#include <ifcpp/IFC4/include/IfcWindowType.h>
#include <ifcpp/IFC4/include/IfcDoor.h>

#include <ifcpp/IFC4/include/IfcRelAssignsToProduct.h>
#include <ifcpp/IFC4/include/IfcRelFillsElement.h>
#include <ifcpp/IFC4/include/IfcRelAssociatesMaterial.h>
#include <ifcpp/IFC4/include/IfcRelDefinesByType.h>


#include "IFCC_Helper.h"

namespace IFCC {

BuildingElement::BuildingElement(int id) :
	EntityBase(id),
	m_surfaceComponent(false),
	m_subSurfaceComponent(false),
	m_constructionId(-1)

{

}

bool BuildingElement::set(std::shared_ptr<IfcElement> ifcElement, ObjectTypes type) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcElement)))
		return false;

	m_type = type;
	m_name = label2s(ifcElement->m_Name);
	for(const auto& relop : ifcElement->m_HasOpenings_inverse) {
		m_containedOpeningsOriginal.push_back(relop.lock()->m_RelatedOpeningElement);
	}
	if(m_type == OT_Roof || m_type == OT_Slab || m_type == OT_Wall)
		m_surfaceComponent = true;
	if(m_type == OT_Window || m_type == OT_Door)
		m_subSurfaceComponent = true;

	if(m_type == OT_Window || m_type == OT_Door) {
		for(const auto& relop : ifcElement->m_FillsVoids_inverse) {
			m_isUsedFromOpeningsOriginal.push_back(relop.lock()->m_RelatingOpeningElement);
		}
		if(m_type == OT_Window) {
			shared_ptr<IfcWindow> window = dynamic_pointer_cast<IfcWindow>(ifcElement);
			if(window != nullptr) {
				m_openingProperties.m_isWindow = true;
				m_openingProperties.m_isDoor = false;
				if(window->m_PredefinedType != nullptr)
					m_openingProperties.m_windowType = window->m_PredefinedType->m_enum;
				if(window->m_OverallHeight != nullptr)
					m_openingProperties.m_windowHeight = window->m_OverallHeight->m_value;
				if(window->m_OverallWidth != nullptr)
					m_openingProperties.m_windowWidth = window->m_OverallWidth->m_value;
				if(window->m_PartitioningType != nullptr)
					m_openingProperties.m_windowPartitionType = window->m_PartitioningType->m_enum;
				m_openingProperties.m_windowUserDefinedPartitionType = label2s(window->m_UserDefinedPartitioningType);
				switch(m_openingProperties.m_windowType) {
					case IfcWindowTypeEnum::ENUM_WINDOW: m_openingProperties.m_name = "Window"; break;
					case IfcWindowTypeEnum::ENUM_SKYLIGHT: m_openingProperties.m_name = "Skylight"; break;
					case IfcWindowTypeEnum::ENUM_LIGHTDOME: m_openingProperties.m_name = "LightDome"; break;
					case IfcWindowTypeEnum::ENUM_USERDEFINED: m_openingProperties.m_name = "UserDefined"; break;
					case IfcWindowTypeEnum::ENUM_NOTDEFINED: m_openingProperties.m_name = "Not defined"; break;
				}
			}
			for(const auto& reltypes : ifcElement->m_IsTypedBy_inverse) {
				shared_ptr<IfcRelDefinesByType> rel_types(reltypes);
				shared_ptr<IfcWindowStyle> windowStyle = dynamic_pointer_cast<IfcWindowStyle>(rel_types->m_RelatingType);
				if(windowStyle != nullptr) {
					if(windowStyle->m_ConstructionType != nullptr) {

					}
					if(windowStyle->m_OperationType != nullptr) {
						IfcWindowStyleOperationEnum::IfcWindowStyleOperationEnumEnum type =
								windowStyle->m_OperationType->m_enum;

						switch(type) {
							case IfcWindowStyleOperationEnum::ENUM_SINGLE_PANEL: m_openingProperties.m_windowConstructionTypes.push_back("Single pane"); break;
							case IfcWindowStyleOperationEnum::ENUM_DOUBLE_PANEL_VERTICAL: m_openingProperties.m_windowConstructionTypes.push_back("Double pane vertical"); break;
							case IfcWindowStyleOperationEnum::ENUM_DOUBLE_PANEL_HORIZONTAL: m_openingProperties.m_windowConstructionTypes.push_back("Double pane horizontal"); break;
							case IfcWindowStyleOperationEnum::ENUM_TRIPLE_PANEL_VERTICAL: m_openingProperties.m_windowConstructionTypes.push_back("Triple pane vertical"); break;
							case IfcWindowStyleOperationEnum::ENUM_TRIPLE_PANEL_BOTTOM: m_openingProperties.m_windowConstructionTypes.push_back("Triple pane bottom"); break;
							case IfcWindowStyleOperationEnum::ENUM_TRIPLE_PANEL_TOP: m_openingProperties.m_windowConstructionTypes.push_back("Triple pane top"); break;
							case IfcWindowStyleOperationEnum::ENUM_TRIPLE_PANEL_LEFT: m_openingProperties.m_windowConstructionTypes.push_back("Triple pane left"); break;
							case IfcWindowStyleOperationEnum::ENUM_TRIPLE_PANEL_RIGHT: m_openingProperties.m_windowConstructionTypes.push_back("Triple pane right"); break;
							case IfcWindowStyleOperationEnum::ENUM_TRIPLE_PANEL_HORIZONTAL: m_openingProperties.m_windowConstructionTypes.push_back("Triple pane horizontal"); break;
							case IfcWindowStyleOperationEnum::ENUM_USERDEFINED: m_openingProperties.m_windowConstructionTypes.push_back("User defined panes"); break;
							case IfcWindowStyleOperationEnum::ENUM_NOTDEFINED: m_openingProperties.m_windowConstructionTypes.push_back("Panes not defined"); break;
						}

					}
				}
				shared_ptr<IfcWindowType> windowType = dynamic_pointer_cast<IfcWindowType>(rel_types);
				if(windowType != nullptr) {
//					if(windowType->m_ConstructionType != nullptr) {

//					}
				}
			}
		}
		// door
		else {
			shared_ptr<IfcDoor> door = dynamic_pointer_cast<IfcDoor>(ifcElement);
			if(door != nullptr) {
				m_openingProperties.m_isWindow = false;
				m_openingProperties.m_isDoor = true;
			}
		}
	}

	for(const auto& relass : ifcElement->m_HasAssociations_inverse) {
		shared_ptr<IfcRelAssociates> rel_associates(relass);
		shared_ptr<IfcRelAssociatesMaterial> associated_material = dynamic_pointer_cast<IfcRelAssociatesMaterial>(rel_associates);
		if (associated_material != nullptr && associated_material->m_RelatingMaterial != nullptr) {
			shared_ptr<IfcMaterialLayerSetUsage> material_layer_set_usage = dynamic_pointer_cast<IfcMaterialLayerSetUsage>(associated_material->m_RelatingMaterial);
			if (material_layer_set_usage != nullptr && material_layer_set_usage->m_ForLayerSet != nullptr) {
				for (size_t jj = 0; jj < material_layer_set_usage->m_ForLayerSet->m_MaterialLayers.size(); ++jj) {
					const shared_ptr<IfcMaterialLayer>& material_layer = material_layer_set_usage->m_ForLayerSet->m_MaterialLayers[jj];
					if (material_layer) {
						const shared_ptr<IfcMaterial>& mat = material_layer->m_Material;					//optional
						if (mat) {
							m_materialLayers.emplace_back(std::pair<double,std::string>(material_layer->m_LayerThickness->m_value, label2s(mat->m_Name)));
						}
					}

				}
			}
		}
	}

	return true;
}

void BuildingElement::update(std::shared_ptr<ProductShapeData> productShape, std::vector<Opening>& openings) {
	transform(productShape);
	fetchGeometry(productShape);
	fetchOpenings(openings);
}


void BuildingElement::transform(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	m_transformMatrix = productShape->getTransform();
	if(m_transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(m_transformMatrix);
	}
}

void BuildingElement::fetchGeometry(std::shared_ptr<ProductShapeData> productShape) {
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

	findSurfacePairs();
}

void BuildingElement::findSurfacePairs() {
	if(m_surfaces.size() < 2)
		return;

	for(int i=0; i<m_surfaces.size()-1; ++i) {
		for(int j=i+1; j<m_surfaces.size(); ++j) {
			if(m_surfaces[i].isParallelTo(m_surfaces[j]))
				m_parallelSurfaces.push_back(std::make_pair(i,j));
		}
	}
}

void BuildingElement::fetchOpenings(std::vector<Opening>& openings) {

	for(const auto& opOrg : m_isUsedFromOpeningsOriginal) {
		for(auto& op : openings) {
			if(op.m_guid == guidFromObject(opOrg.get())) {
				m_usedFromOpenings.push_back(op.m_id);
				op.m_openingElementIds.push_back(m_id);
				break;
			}
		}
	}

	for(const auto& opOrg : m_containedOpeningsOriginal) {
		for(auto& op : openings) {
			if(op.m_guid == guidFromObject(opOrg.get())) {
				m_containedOpenings.push_back(op.m_id);
				op.m_containedInElementIds.push_back(m_id);
				break;
			}
		}
	}

	if(m_subSurfaceComponent) {
		m_openingProperties.m_usedInConstructionIds.clear();
		for(const auto& opId : m_usedFromOpenings) {
			auto fit = std::find_if(openings.begin(), openings.end(),
									[opId](const auto& op) {return op.m_id == opId; });
			if(fit != openings.end()) {
				int elemCount = fit->m_containedInElementIds.size();
				m_openingProperties.m_usedInConstructionIds.insert(m_openingProperties.m_usedInConstructionIds.end(),
																   fit->m_containedInElementIds.begin(),
																   fit->m_containedInElementIds.end());
			}
		}
	}
}


const meshVector_t& BuildingElement::meshVector() const {
	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
	}

	if(!m_meshSetClosedFinal.empty())
		return m_meshSetClosedFinal;

	return m_meshSetOpenFinal;
}

const polyVector_t& BuildingElement::polyVector() const {
	if(!m_polyvectClosedFinal.empty() && !m_polyvectOpenFinal.empty()) {
		throw IBK::Exception("Mesh includes open and closed meshset.", "Space::meshVector");
	}

	if(!m_polyvectClosedFinal.empty())
		return m_polyvectClosedFinal;

	return m_polyvectOpenFinal;
}

const std::vector<Surface>& BuildingElement::surfaces() const {
	return m_surfaces;
}

carve::mesh::Face<3>* BuildingElement::face(FaceIndex findex) const {
	if(!m_meshSetClosedFinal.empty() && !m_meshSetOpenFinal.empty()) {
		return nullptr;
	}

	if(!m_meshSetClosedFinal.empty())
		return faceFromMeshset(m_meshSetClosedFinal, findex);

	return faceFromMeshset(m_meshSetOpenFinal, findex);
}

double	BuildingElement::thickness() const {
	if(m_materialLayers.empty()) {
		if(m_parallelSurfaces.empty())
			return 0;

		double minDist = 10001;
		for(int epi=0; epi<m_parallelSurfaces.size(); ++epi) {
			const Surface& surf1 = m_surfaces[m_parallelSurfaces[epi].first];
			const Surface& surf2 = m_surfaces[m_parallelSurfaces[epi].second];
			minDist = std::min(minDist, surf1.distanceToParallelPlane(surf2.planeNormal()));
		}
		if(minDist > 10000)
			return 0;
		return minDist;
	}

	double res = 0;
	for(size_t i=0; i<m_materialLayers.size(); ++i) {
		res += m_materialLayers[i].first;
	}
	return res;
}


} // namespace IFCC
