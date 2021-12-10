#include "IFCC_SpaceBoundary.h"

#include <ifcpp/IFC4/include/IfcConnectionCurveGeometry.h>
#include <ifcpp/IFC4/include/IfcConnectionPointEccentricity.h>
#include <ifcpp/IFC4/include/IfcConnectionPointGeometry.h>
#include <ifcpp/IFC4/include/IfcConnectionSurfaceGeometry.h>
#include <ifcpp/IFC4/include/IfcConnectionVolumeGeometry.h>
#include <ifcpp/IFC4/include/IfcSurface.h>
#include <ifcpp/IFC4/include/IfcFaceBasedSurfaceModel.h>
#include <ifcpp/IFC4/include/IfcFaceSurface.h>

#include "IFCC_Helper.h"
#include "IFCC_Types.h"
#include "IFCC_RepresentationConverter.h"

namespace IFCC {

SpaceBoundary::SpaceBoundary(int id) :
	EntityBase(id),
	m_typeRelatedElement(OT_All),
	m_physicalOrVirtual(IfcPhysicalOrVirtualEnum::ENUM_NOTDEFINED),
	m_internalOrExternal(IfcInternalOrExternalEnum::ENUM_NOTDEFINED),
	m_type(CT_Unknown),
	m_elementEntityId(-1),
	m_openingId(-1)
{

}

bool SpaceBoundary::set(std::shared_ptr<IfcRelSpaceBoundary> ifcSpaceBoundary) {
	if(!EntityBase::set(dynamic_pointer_cast<IfcRoot>(ifcSpaceBoundary)))
		return false;

	if(ifcSpaceBoundary->m_RelatedBuildingElement != nullptr) {
		m_guidRelatedElement = guidFromObject(ifcSpaceBoundary->m_RelatedBuildingElement.get());
		m_nameRelatedElement = label2s(ifcSpaceBoundary->m_RelatedBuildingElement->m_Name);
	}

	if(ifcSpaceBoundary->m_PhysicalOrVirtualBoundary != nullptr) {
		m_physicalOrVirtual = ifcSpaceBoundary->m_PhysicalOrVirtualBoundary->m_enum;
		if(m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL)
			return false;
	}
	if(ifcSpaceBoundary->m_InternalOrExternalBoundary != nullptr) {
		m_internalOrExternal = ifcSpaceBoundary->m_InternalOrExternalBoundary->m_enum;
	}

	m_connectionGeometry = ifcSpaceBoundary->m_ConnectionGeometry;
	return true;
}


bool SpaceBoundary::set(const std::string& name, const BuildingElement& elem) {
	m_name = name;
	setRelatingElementType(elem.type());
	m_physicalOrVirtual = IfcPhysicalOrVirtualEnum::ENUM_PHYSICAL;
	m_guidRelatedElement = elem.m_guid;
	m_nameRelatedElement = elem.m_name;
	return true;
}

void SpaceBoundary::setRelatingElementType(ObjectTypes type) {
	m_typeRelatedElement = type;
	if(type == OT_Wall || type == OT_CurtainWall || type == OT_Roof || type == OT_Slab)
		m_type = CT_ConstructionElement;
	else if(type == OT_Door || type == OT_Window)
		m_type = CT_OpeningElement;
	else m_type = CT_Others;
}

bool SpaceBoundary::fetchGeometry(shared_ptr<UnitConverter>& unit_converter, const carve::math::Matrix& spaceTransformation) {
	// connection geometry is set from IFCSpaceBoundary
	if(m_connectionGeometry != nullptr) {
		// get geometry data from connection geometry by conversion via ItemShapeData
		shared_ptr<ItemShapeData> item_data(new ItemShapeData);
		std::shared_ptr<IfcConnectionCurveGeometry> curveGeom = std::dynamic_pointer_cast<IfcConnectionCurveGeometry>(m_connectionGeometry);
		if(curveGeom != nullptr) {
			///< \todo Implement
		}
		std::shared_ptr<IfcConnectionPointEccentricity> pointEccGeom = std::dynamic_pointer_cast<IfcConnectionPointEccentricity>(m_connectionGeometry);
		if(pointEccGeom != nullptr) {
			///< \todo Implement
		}
		std::shared_ptr<IfcConnectionPointGeometry> pointGeom = std::dynamic_pointer_cast<IfcConnectionPointGeometry>(m_connectionGeometry);
		if(pointGeom != nullptr) {
			///< \todo Implement
		}
		std::shared_ptr<IfcConnectionSurfaceGeometry> surfaceGeom = std::dynamic_pointer_cast<IfcConnectionSurfaceGeometry>(m_connectionGeometry);
		if(surfaceGeom != nullptr) {
			shared_ptr<IfcSurfaceOrFaceSurface> surface = surfaceGeom->m_SurfaceOnRelatingElement;
			if(surface != nullptr) {
				std::shared_ptr<IfcFaceBasedSurfaceModel> fbSurface = std::dynamic_pointer_cast<IfcFaceBasedSurfaceModel>(surface);
				if(fbSurface != nullptr) {
					///< \todo Implement
				}
				std::shared_ptr<IfcFaceSurface> fSurface = std::dynamic_pointer_cast<IfcFaceSurface>(surface);
				if(fSurface != nullptr) {
					///< \todo Implement
				}
				std::shared_ptr<IfcSurface> nSurface = std::dynamic_pointer_cast<IfcSurface>(surface);
				if(nSurface != nullptr) {
					shared_ptr<GeometrySettings> geom_settings = shared_ptr<GeometrySettings>( new GeometrySettings() );
					RepresentationConverter repConvert(geom_settings, unit_converter);
					repConvert.convertIfcGeometricRepresentationItem(nSurface,item_data);
					if(spaceTransformation != carve::math::Matrix::IDENT()) {
						item_data->applyTransformToItem(spaceTransformation);
					}
				}
			}
		}
		std::shared_ptr<IfcConnectionVolumeGeometry> volumeGeom = std::dynamic_pointer_cast<IfcConnectionVolumeGeometry>(m_connectionGeometry);
		if(volumeGeom != nullptr) {
			///< \todo Implement
		}

		// geometry is converted and transformed into carve::MeshSet<3>
		// now transform it into IBKMK::Vector3D
		meshVector_t meshSetClosedFinal = item_data->m_meshsets;
		meshVector_t meshSetOpenFinal = item_data->m_meshsets_open;
		if(meshSetClosedFinal.empty() && meshSetOpenFinal.empty())
			return false;

		// try to simplify meshes by merging all coplanar faces
		meshVector_t& currentMeshSets =  meshSetClosedFinal.empty() ? meshSetOpenFinal : meshSetClosedFinal;
		if(!currentMeshSets.empty()) {
			simplifyMesh(currentMeshSets, false);
		}

		if(!meshSetClosedFinal.empty()) {
			int msCount = meshSetClosedFinal.size();
			for(int i=0; i<msCount; ++i) {
				m_polyvectClosedFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
				const carve::mesh::MeshSet<3>& currMeshSet = *meshSetClosedFinal[i];
				convert(currMeshSet, m_polyvectClosedFinal.back());
			}
		}
		if(!meshSetOpenFinal.empty()) {
			int msCount = meshSetOpenFinal.size();
			for(int i=0; i<msCount; ++i) {
				m_polyvectOpenFinal.push_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
				const carve::mesh::MeshSet<3>& currMeshSet = *meshSetOpenFinal[i];
				convert(currMeshSet, m_polyvectOpenFinal.back());
			}
		}
		void createSurfaceVect();
		return true;
	}

	// no geometry from IFC element
	// geometry must be set from related building element and space
	// use other fetchGeometry function
	else {
		return false;
	}
}

bool SpaceBoundary::fetchGeometry(const Surface& surface) {
	m_polyvectClosedFinal.clear();
	m_polyvectOpenFinal.clear();
	if(surface.m_valid) {
		m_polyvectClosedFinal.emplace_back(std::vector<std::vector<std::vector<IBKMK::Vector3D>>>());
		m_polyvectClosedFinal.back().emplace_back(std::vector<std::vector<IBKMK::Vector3D>>());
		m_polyvectClosedFinal.back().back().emplace_back(surface.polygon());
		createSurfaceVect();
		return true;
	}
	return false;
}

static void setSurfaceType(Surface& surf, IfcInternalOrExternalEnum::IfcInternalOrExternalEnumEnum type) {
	switch(type) {
		case IfcInternalOrExternalEnum::ENUM_INTERNAL: surf.m_positionType = Surface::PT_Internal; break;
		case IfcInternalOrExternalEnum::ENUM_EXTERNAL: surf.m_positionType = Surface::PT_External; break;
		case IfcInternalOrExternalEnum::ENUM_EXTERNAL_EARTH: surf.m_positionType = Surface::PT_External_Ground; break;
		case IfcInternalOrExternalEnum::ENUM_EXTERNAL_WATER:
		case IfcInternalOrExternalEnum::ENUM_EXTERNAL_FIRE:
		case IfcInternalOrExternalEnum::ENUM_NOTDEFINED: surf.m_positionType = Surface::PT_Unknown; break;
	}
}

void SpaceBoundary::createSurfaceVect() {
	m_surfaces.clear();
	std::string name = m_nameRelatedElement;
	if(name.empty())
		name = std::to_string(m_id);
	for(const auto& polyvect3 : m_polyvectClosedFinal) {
		for(const auto& polyvect2 : polyvect3) {
			for(const auto& polyvect1 : polyvect2) {
				if(!polyvect1.empty()) {
					m_surfaces.emplace_back(Surface(polyvect1));
					m_surfaces.back().m_elementEntityId = m_elementEntityId;
					m_surfaces.back().m_openingId = m_openingId;
					m_surfaces.back().m_name = name;
					m_surfaces.back().m_id = GUID_maker::instance().guid();
					m_surfaces.back().m_virtualSurface = m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL;
					setSurfaceType(m_surfaces.back(), m_internalOrExternal);
				}
			}
		}
	}
	for(const auto& polyvect3 : m_polyvectOpenFinal) {
		for(const auto& polyvect2 : polyvect3) {
			for(const auto& polyvect1 : polyvect2) {
				if(!polyvect1.empty()) {
					m_surfaces.emplace_back(Surface(polyvect1));
					m_surfaces.back().m_elementEntityId = m_elementEntityId;
					m_surfaces.back().m_openingId = m_openingId;
					m_surfaces.back().m_name = name;
					m_surfaces.back().m_id = GUID_maker::instance().guid();
					m_surfaces.back().m_virtualSurface = m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL;
					setSurfaceType(m_surfaces.back(), m_internalOrExternal);
				}
			}
		}
	}
}


} // namespace IFCC
