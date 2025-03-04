#include "IFCC_Opening.h"

#include <ifcpp/IFC4X3/include/IfcGloballyUniqueId.h>

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>
#include <Carve/src/include/carve/carve.hpp>

#include <IBKMK_3DCalculations.h>

#include "IFCC_GeometryInputData.h"
#include "IFCC_MeshUtils.h"
#include "IFCC_Helper.h"
#include "IFCC_BuildingElement.h"
#include "IFCC_RepresentationHelper.h"
#include "IFCC_CSG_Adapter.h"

namespace IFCC {

Opening::Opening(int id) :
	EntityBase(id)
{

}

bool Opening::set(std::shared_ptr<IFC4X3::IfcFeatureElementSubtraction> ifcElement) {
	if(!EntityBase::set(dynamic_pointer_cast<IFC4X3::IfcRoot>(ifcElement)))
		return false;

	m_guid = guidFromObject(ifcElement.get());

	return true;
}

void Opening::update(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors) {
	transform(productShape);
	fetchGeometry(productShape, errors);
}


void Opening::transform(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	carve::math::Matrix transformMatrix = productShape->getTransform();
	if(transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(transformMatrix, true, true);
	}
}

void Opening::fetchGeometry(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors) {
	if(productShape == nullptr)
		return;

	m_originalMesh = surfacesFromRepresentation(productShape, m_surfaces, errors, OT_Opening, m_id);
}

const std::vector<int>& Opening::openingElementIds() const {
	return m_openingElementIds;
}

void Opening::checkSurfaceType(const BuildingElement &element, double eps) {
	int intersections = 0;
	for(const Surface& elemSurface : element.surfaces()) {
		for(Surface& opSurface : m_surfaces) {
			if(opSurface.sideType() != Surface::ST_Unknown)
				continue;

			bool parallel = elemSurface.isParallelTo(opSurface, eps);
			bool intersected = false;
			if(elemSurface.isValid(eps) && opSurface.isValid(eps)) {
				if(IBKMK::polyIntersect(elemSurface.polygon(), opSurface.polygon()))
					intersected = true;
			}
			if(intersected && !parallel) {
				opSurface.setSideType(Surface::ST_UnProbableSide);
				++intersections;
			}
		}
	}

	if(!element.m_possibleSideSurfaces.empty()) {
		for(const auto& sideIndex : element.m_possibleSideSurfaces) {
			const Surface& surf = element.surfaces()[sideIndex];
			for(Surface& opSurface : m_surfaces) {
				if(opSurface.sideType() != Surface::ST_Unknown)
					continue;
				if(surf.isParallelTo(opSurface, eps))
					opSurface.setSideType(Surface::ST_ProbableSide);
			}
		}

	}


//	if(intersections == 0)
	//		return;
}

void Opening::createCSGSurfaces(const BuildingElement &element) {
//	double totalVolume = 0;
//	for(auto meshset : m_originalMesh) {
//		double vol = MeshUtils::getMeshVolume(meshset.get());
//		totalVolume += vol;
//	}

	// create 3D intersection of opening and building element
	if(!element.m_originalMesh.empty() && !m_originalMesh.empty()) {
		shared_ptr<carve::mesh::MeshSet<3> > resultMesh;
		shared_ptr<carve::mesh::MeshSet<3> > firstMesh = element.m_originalMesh.front();
		//	meshSets.erase(meshSets.begin());
		shared_ptr<GeometrySettings> geom_settings = shared_ptr<GeometrySettings>( new GeometrySettings() );
		meshVector_t resultVect;
		try {
			CSG_Adapter::computeCSG(firstMesh, m_originalMesh, carve::csg::CSG::INTERSECTION, resultMesh, geom_settings);
			if(resultMesh) {
				resultVect.push_back(resultMesh);
				surfacesFromMeshSets(resultVect, m_surfacesCSG);

//				double totalVolumeRes = 0;
//				for(auto meshset : resultVect) {
//					double vol = MeshUtils::getMeshVolume(meshset.get());
//					totalVolumeRes += vol;
//				}
//				bool smallerVolume = false;
//				if(totalVolumeRes >= totalVolume) {
//					smallerVolume = true;
//				}
			}
		}
		catch (...) {
		}
	}
}

const std::vector<Surface>& Opening::surfaces() const {
	return m_surfaces;
}

const std::vector<Surface> &Opening::surfacesCSG() const {
	return m_surfacesCSG;
}

std::string Opening::guid() const {
	return m_guid;
}

void Opening::addOpeningElementId(int id) {
	m_openingElementIds.push_back(id);
}

void Opening::addContainingElementId(int id) {
	m_containedInElementIds.push_back(id);
}

void Opening::insertContainingElementId(std::vector<int>& other) const {
	if(!m_containedInElementIds.empty()) {
		other.insert(other.end(), m_containedInElementIds.begin(), m_containedInElementIds.end());
	}
}

void Opening::setSpaceBoundary(std::shared_ptr<SpaceBoundary> sb) {
	m_spaceBoundary = sb;
}

bool Opening::hasSpaceBoundary() const {
	return m_spaceBoundary.get() != nullptr;
}

} // namespace IFCC
