#include "IFCC_Opening.h"

#include <ifcpp/IFC4X3/include/IfcGloballyUniqueId.h>
#include <ifcpp/geometry/GeometryInputData.h>

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>
#include <Carve/src/include/carve/carve.hpp>
#include <ifcpp/geometry/MeshUtils.h>

#include "IFCC_Helper.h"

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

	surfacesFromRepresentation(productShape, m_surfaces, errors, OT_Opening, m_id);
}

const std::vector<int>& Opening::openingElementIds() const {
	return m_openingElementIds;
}

const std::vector<Surface>& Opening::surfaces() const {
	return m_surfaces;
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
