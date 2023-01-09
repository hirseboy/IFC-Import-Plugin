#include "IFCC_Opening.h"

#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>

#include <IBKMK_Vector3D.h>

#include <carve/mesh.hpp>
#include <carve/matrix.hpp>

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

	carve::math::Matrix transformMatrix = productShape->getTransform();
	if(transformMatrix != carve::math::Matrix::IDENT()) {
		productShape->applyTransformToProduct(transformMatrix);
	}
}

void Opening::fetchGeometry(std::shared_ptr<ProductShapeData> productShape) {
	if(productShape == nullptr)
		return;

	surfacesFromRepresentation(productShape, m_surfaces);
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
