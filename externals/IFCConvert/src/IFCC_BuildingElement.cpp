#include "IFCC_BuildingElement.h"

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>

#include <ifcpp/IFC4/include/IfcRelVoidsElement.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcObjectTypeEnum.h>
#include <ifcpp/IFC4/include/IfcMaterialLayerSetUsage.h>
#include <ifcpp/IFC4/include/IfcMaterialLayerSet.h>
#include <ifcpp/IFC4/include/IfcMaterialLayer.h>
#include <ifcpp/IFC4/include/IfcMaterial.h>
#include <ifcpp/IFC4/include/IfcMaterialDefinitionRepresentation.h>
#include <ifcpp/IFC4/include/IfcMaterialList.h>
#include <ifcpp/IFC4/include/IfcMaterialProfile.h>
#include <ifcpp/IFC4/include/IfcMaterialProfileSet.h>

#include <ifcpp/IFC4/include/IfcNonNegativeLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcPositiveLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcWindowStyle.h>
#include <ifcpp/IFC4/include/IfcWindowStyleOperationEnum.h>
#include <ifcpp/IFC4/include/IfcWindowType.h>
#include <ifcpp/IFC4/include/IfcDoorStyle.h>

#include <ifcpp/IFC4/include/IfcRelAssignsToProduct.h>
#include <ifcpp/IFC4/include/IfcRelFillsElement.h>
#include <ifcpp/IFC4/include/IfcRelAssociatesMaterial.h>
#include <ifcpp/IFC4/include/IfcRelDefinesByType.h>
#include <ifcpp/IFC4/include/IfcRelDefinesByProperties.h>
#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcObjectDefinition.h>

#include <ifcpp/IFC4/include/IfcPropertySetDefinitionSelect.h>
#include <ifcpp/IFC4/include/IfcPropertySetDefinition.h>
#include <ifcpp/IFC4/include/IfcPropertySet.h>
#include <ifcpp/IFC4/include/IfcProperty.h>
#include <ifcpp/IFC4/include/IfcPropertySingleValue.h>
#include <ifcpp/IFC4/include/IfcDerivedMeasureValue.h>
#include <ifcpp/IFC4/include/IfcMeasureValue.h>
#include <ifcpp/IFC4/include/IfcSimpleValue.h>
#include <ifcpp/IFC4/include/IfcBinary.h>
#include <ifcpp/IFC4/include/IfcBoolean.h>
#include <ifcpp/IFC4/include/IfcDate.h>
#include <ifcpp/IFC4/include/IfcDateTime.h>
#include <ifcpp/IFC4/include/IfcDuration.h>
#include <ifcpp/IFC4/include/IfcReal.h>
#include <ifcpp/IFC4/include/IfcPositiveInteger.h>
#include <ifcpp/IFC4/include/IfcTime.h>
#include <ifcpp/IFC4/include/IfcTimeStamp.h>
#include <ifcpp/IFC4/include/IfcLogical.h>

#include <ifcpp/IFC4/include/IfcMaterialProperties.h>



#include "IFCC_Helper.h"

namespace IFCC {

void BuildingElement::WallProperties::update(std::shared_ptr<IfcWall>& ifcWall) {
	if(!ifcWall)
		return;
	if(ifcWall->m_PredefinedType)
		m_wallType = ifcWall->m_PredefinedType->m_enum;
}

BuildingElement::BuildingElement(int id) :
	EntityBase(id),
	m_constructionId(-1),
	m_type(BET_All),
	m_surfaceComponent(false),
	m_subSurfaceComponent(false)

{

}

bool BuildingElement::set(std::shared_ptr<IfcElement> ifcElement, BuildingElementTypes type) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcElement)))
		return false;

	m_type = type;
	m_name = label2s(ifcElement->m_Name);
	for(const auto& relop : ifcElement->m_HasOpenings_inverse) {
		m_containedOpeningsOriginal.push_back(relop.lock()->m_RelatedOpeningElement);
	}
	if(isConstructionType(m_type) || isConstructionSimilarType(m_type))
		m_surfaceComponent = true;
	else if(m_type == BET_Window || m_type == BET_Door)
		m_subSurfaceComponent = true;

	// look for properties
	for(const auto& relproperties : ifcElement->m_IsDefinedBy_inverse) {
		shared_ptr<IfcRelDefinesByProperties> rel_properties(relproperties);
		if(rel_properties && rel_properties->m_RelatingPropertyDefinition) {
			shared_ptr<IfcPropertySetDefinition> property_set_def = dynamic_pointer_cast<IfcPropertySetDefinition>(rel_properties->m_RelatingPropertyDefinition);
			if( property_set_def ) {
				shared_ptr<IfcPropertySet> property_set = dynamic_pointer_cast<IfcPropertySet>(property_set_def);
				if( property_set ) {
					std::string pset_name = label2s(property_set->m_Name);
					for(const auto& property : property_set->m_HasProperties) {
						std::string name = name2s(property->m_Name);
						bool usesThisProperty = Property::relevantProperty(pset_name,name);
						if(usesThisProperty) {
							Property prop;
							prop.m_name = name;
							getProperty(property,pset_name, prop);
							std::map<std::string, Property> inner;
							inner.insert(std::make_pair(name, prop));
							m_propertyMap.insert(std::make_pair(pset_name, inner));
						}
					}
				}
			}
		}
	}
	setThermalTransmittance();

	if(isOpeningType(m_type)) {
		for(const auto& relop : ifcElement->m_FillsVoids_inverse) {
			m_isUsedFromOpeningsOriginal.push_back(relop.lock()->m_RelatingOpeningElement);
		}
		if(m_type == BET_Window) {
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
					case IfcWindowTypeEnum::ENUM_WINDOW: m_openingProperties.m_typeName = "Window"; break;
					case IfcWindowTypeEnum::ENUM_SKYLIGHT: m_openingProperties.m_typeName = "Skylight"; break;
					case IfcWindowTypeEnum::ENUM_LIGHTDOME: m_openingProperties.m_typeName = "LightDome"; break;
					case IfcWindowTypeEnum::ENUM_USERDEFINED: m_openingProperties.m_typeName = "UserDefined"; break;
					case IfcWindowTypeEnum::ENUM_NOTDEFINED: m_openingProperties.m_typeName = "Not defined"; break;
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
				if(door->m_PredefinedType != nullptr)
					m_openingProperties.m_doorType = door->m_PredefinedType->m_enum;
				if(door->m_OverallHeight != nullptr)
					m_openingProperties.m_doorHeight = door->m_OverallHeight->m_value;
				if(door->m_OverallWidth != nullptr)
					m_openingProperties.m_doorWidth = door->m_OverallWidth->m_value;
				for(const auto& reltypes : ifcElement->m_IsTypedBy_inverse) {
					shared_ptr<IfcRelDefinesByType> rel_types(reltypes);
					shared_ptr<IfcDoorStyle> doorStyle = dynamic_pointer_cast<IfcDoorStyle>(rel_types->m_RelatingType);
					if(doorStyle != nullptr) {

					}
				}
			}
		}
	}

	if(m_type == BET_Wall) {
		shared_ptr<IfcWall> wall = dynamic_pointer_cast<IfcWall>(ifcElement);
		m_wallProperties.update(wall);

		if(!wall->m_IsDecomposedBy_inverse.empty()) {
			for(size_t i=0; i<wall->m_IsDecomposedBy_inverse.size(); ++i) {
				shared_ptr<IfcRelAggregates> relAggregate(wall->m_IsDecomposedBy_inverse[i]);
				if(relAggregate) {
					for(size_t io=0; io<relAggregate->m_RelatedObjects.size(); ++io) {
						const shared_ptr<IfcObjectDefinition>& object = relAggregate->m_RelatedObjects[io];

					}
				}
			}
		}
		if(!wall->m_Decomposes_inverse.empty()) {
			for(size_t i=0; i<wall->m_Decomposes_inverse.size(); ++i) {
				shared_ptr<IfcRelAggregates> relAggregate(wall->m_Decomposes_inverse[i]);
				if(relAggregate) {

				}
			}
		}
	}

	for(const auto& relass : ifcElement->m_HasAssociations_inverse) {
		shared_ptr<IfcRelAssociates> rel_associates(relass);
		shared_ptr<IfcRelAssociatesMaterial> associated_material = dynamic_pointer_cast<IfcRelAssociatesMaterial>(rel_associates);
		if (associated_material != nullptr && associated_material->m_RelatingMaterial != nullptr) {
			std::string classname = associated_material->m_RelatingMaterial->className();

			shared_ptr<IfcMaterialDefinitionRepresentation> material_definition_rep = dynamic_pointer_cast<IfcMaterialDefinitionRepresentation>(associated_material->m_RelatingMaterial);
			if(material_definition_rep) {
				const shared_ptr<IfcMaterial>& mat = material_definition_rep->m_RepresentedMaterial;
				if (mat) {
					m_materialLayers.emplace_back(std::pair<double,std::string>(0.01, label2s(mat->m_Name)));
					m_materialPropertyMap.emplace_back(std::map<std::string,std::map<std::string,Property>>());
					getMaterialProperties(mat, m_materialPropertyMap.back());
				}
			}

			shared_ptr<IfcMaterialDefinition> material_definition = dynamic_pointer_cast<IfcMaterialDefinition>(associated_material->m_RelatingMaterial);
			if(material_definition) {
				shared_ptr<IfcMaterial> mat = dynamic_pointer_cast<IfcMaterial>(material_definition);
				if (mat) {
					m_materialLayers.emplace_back(std::pair<double,std::string>(0.01, label2s(mat->m_Name)));
					m_materialPropertyMap.emplace_back(std::map<std::string,std::map<std::string,Property>>());
					getMaterialProperties(mat, m_materialPropertyMap.back());
				}
				shared_ptr<IfcMaterialLayer> matLayer = dynamic_pointer_cast<IfcMaterialLayer>(material_definition);
				if (matLayer) {
					const shared_ptr<IfcMaterial>& mat = matLayer->m_Material;					//optional
					if (mat) {
						m_materialLayers.emplace_back(std::pair<double,std::string>(matLayer->m_LayerThickness->m_value, label2s(mat->m_Name)));
						m_materialPropertyMap.emplace_back(std::map<std::string,std::map<std::string,Property>>());
						getMaterialProperties(mat, m_materialPropertyMap.back());
					}
				}
				shared_ptr<IfcMaterialLayerSet> matLayerSet = dynamic_pointer_cast<IfcMaterialLayerSet>(material_definition);
				shared_ptr<IfcMaterialProfile> matProfile = dynamic_pointer_cast<IfcMaterialProfile>(material_definition);
				shared_ptr<IfcMaterialProfileSet> matProfileSet = dynamic_pointer_cast<IfcMaterialProfileSet>(material_definition);

			}
			shared_ptr<IfcMaterialList> material_list = dynamic_pointer_cast<IfcMaterialList>(associated_material->m_RelatingMaterial);
			if(material_list) {
				for(size_t im=0; im<material_list->m_Materials.size(); ++im) {
					const shared_ptr<IfcMaterial>& mat = material_list->m_Materials[im];
					if (mat) {
						m_materialLayers.emplace_back(std::pair<double,std::string>(0.01, label2s(mat->m_Name)));
						m_materialPropertyMap.emplace_back(std::map<std::string,std::map<std::string,Property>>());
						getMaterialProperties(mat, m_materialPropertyMap.back());
					}
				}
			}
			shared_ptr<IfcMaterialUsageDefinition> material_usage_definition = dynamic_pointer_cast<IfcMaterialUsageDefinition>(associated_material->m_RelatingMaterial);
			if(material_usage_definition) {
				shared_ptr<IfcMaterialLayerSetUsage> material_layer_set_usage = dynamic_pointer_cast<IfcMaterialLayerSetUsage>(material_usage_definition);
				if (material_layer_set_usage != nullptr && material_layer_set_usage->m_ForLayerSet != nullptr) {
					for (size_t jj = 0; jj < material_layer_set_usage->m_ForLayerSet->m_MaterialLayers.size(); ++jj) {
						const shared_ptr<IfcMaterialLayer>& material_layer = material_layer_set_usage->m_ForLayerSet->m_MaterialLayers[jj];
						if (material_layer) {
							const shared_ptr<IfcMaterial>& mat = material_layer->m_Material;					//optional
							if (mat) {
								m_materialLayers.emplace_back(std::pair<double,std::string>(material_layer->m_LayerThickness->m_value, label2s(mat->m_Name)));
								m_materialPropertyMap.emplace_back(std::map<std::string,std::map<std::string,Property>>());
								getMaterialProperties(mat, m_materialPropertyMap.back());
							}
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

	carve::math::Matrix transformMatrix = productShape->getTransform();
	if(transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(transformMatrix);
	}
}

void BuildingElement::fetchGeometry(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	if(m_subSurfaceComponent) {
		int g = 0;
	}

	surfacesFromRepresentation(productShape, m_surfaces);

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
			std::string guid = guidFromObject(opOrg.get());
			if(op.guid() == guid) {
				m_usedFromOpenings.push_back(op.m_id);
				op.addOpeningElementId(m_id);
				break;
			}
		}
	}

	for(const auto& opOrg : m_containedOpeningsOriginal) {
		for(auto& op : openings) {
			std::string guid = guidFromObject(opOrg.get());
			if(op.guid() == guid) {
				m_containedOpenings.push_back(op.m_id);
				op.addContainingElementId(m_id);
				break;
			}
		}
	}

	// check openings
//	for(const auto& op : m_containedOpenings) {
//		auto fit = std::find_if(
//					   openings.begin(),
//					   openings.end(),
//					   [op](const auto& opening) {return opening.second.m_id == op; });
//		if(fit == openings.end())
//			continue;

//		std::vector<Surface> diffSurfaces;
//		for(const Surface& elemSurface : m_surfaces) {
//			for(const Surface& opSurface : fit->surfaces()) {
//				bool parallel = elemSurface.isParallelTo(opSurface);
//				if(parallel) {
//					std::vector<Surface> differences = elemSurface.difference(opSurface);
//					if(differences.empty()) {
//						diffSurfaces.push_back(elemSurface);
//					}
//				}
//			}
//		}

//	}
}

const std::vector<Surface>& BuildingElement::surfaces() const {
	return m_surfaces;
}

double	BuildingElement::thickness() const {
	if(m_materialLayers.empty()) {
		if(m_parallelSurfaces.empty())
			return 0;

		double minDist = 10001;
		for(int epi=0; epi<m_parallelSurfaces.size(); ++epi) {
			const Surface& surf1 = m_surfaces[m_parallelSurfaces[epi].first];
			const Surface& surf2 = m_surfaces[m_parallelSurfaces[epi].second];
			minDist = std::min(minDist, surf1.distanceToParallelPlane(surf2));
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

double BuildingElement::openingArea() const {
	if(!isSubSurfaceComponent())
		return 0;

	if(m_openingProperties.m_isWindow) {
		return m_openingProperties.m_windowHeight * m_openingProperties.m_windowWidth;
	}
	if(m_openingProperties.m_isDoor) {
		return m_openingProperties.m_doorHeight * m_openingProperties.m_doorWidth;
	}

	return 0;
}

void BuildingElement::setContainingElements(const std::vector<Opening>& openings) {
	if(!isSubSurfaceComponent())
		return;

	m_openingProperties.m_usedInConstructionIds.clear();
	for(const auto& opId : m_usedFromOpenings) {
		auto fit = std::find_if(openings.begin(), openings.end(),
								[opId](const auto& op) {return op.m_id == opId; });
		if(fit != openings.end()) {
			fit->insertContainingElementId(m_openingProperties.m_usedInConstructionIds);
		}
	}
}

void BuildingElement::setContainedConstructionThickesses(const std::vector<std::shared_ptr<BuildingElement>>& elements) {
	if(!isSubSurfaceComponent())
		return;

	int constructionIDCount = m_openingProperties.m_usedInConstructionIds.size();
	for(int i=0; i<constructionIDCount; ++i) {
		int constId = m_openingProperties.m_usedInConstructionIds[i];
		auto fit = std::find_if(elements.begin(), elements.end(),
								[constId](const auto& constr) {return constr->m_id == constId; });
		if(fit != elements.end()) {
			m_openingProperties.m_constructionThicknesses.push_back((*fit)->thickness());
		}
	}
}

void BuildingElement::setThermalTransmittance() {
	double tt = 0;
	if(m_type == BET_Wall && getDoubleProperty(m_propertyMap, "Pset_WallCommon", "ThermalTransmittance", tt))
		m_thermalTransmittance = tt;
	if(m_type == BET_Window && getDoubleProperty(m_propertyMap, "Pset_WindowCommon", "ThermalTransmittance", tt))
		m_thermalTransmittance = tt;
	if(m_type == BET_Door && getDoubleProperty(m_propertyMap, "Pset_DoorCommon", "ThermalTransmittance", tt))
		m_thermalTransmittance = tt;
}


} // namespace IFCC
