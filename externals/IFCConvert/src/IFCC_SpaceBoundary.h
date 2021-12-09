#ifndef IFCC_SpaceBoundaryH
#define IFCC_SpaceBoundaryH

#include "IFCC_EntityBase.h"

#include <ifcpp/IFC4/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4/include/IfcPhysicalOrVirtualEnum.h>
#include <ifcpp/IFC4/include/IfcInternalOrExternalEnum.h>

#include <ifcpp/model/UnitConverter.h>

#include "IFCC_BuildingElement.h"

namespace IFCC {

class SpaceBoundary : public IFCC::EntityBase
{
public:
	enum ConstructionType {
		CT_OpeningElement,
		CT_ConstructionElement,
		CT_Others,
		CT_Unknown
	};


	explicit SpaceBoundary(int id);

	bool set(std::shared_ptr<IfcRelSpaceBoundary> ifcSpaceBoundary);

	bool set(const std::string& name, const BuildingElement& elem);

	void setRelatingElementType(ObjectTypes type);

	bool fetchGeometry(shared_ptr<UnitConverter>& unit_converter, const carve::math::Matrix& spaceTransformation);

	bool fetchGeometry(const Surface& surface);

	const std::vector<Surface>& surfaces() const {
		return m_surfaces;
	}

	std::string													m_objectTypeRelatedElement;
	std::string													m_guidRelatedElement;
	std::string													m_nameRelatedElement;
	ObjectTypes													m_typeRelatedElement;
	IfcPhysicalOrVirtualEnum::IfcPhysicalOrVirtualEnumEnum		m_physicalOrVirtual;
	IfcInternalOrExternalEnum::IfcInternalOrExternalEnumEnum	m_internalOrExternal;
	ConstructionType											m_type;
	int															m_elementEntityId;
	int															m_openingId;

private:
	void createSurfaceVect();

	polyVector_t												m_polyvectClosedFinal;
	polyVector_t												m_polyvectOpenFinal;
	std::vector<Surface>										m_surfaces;
	std::shared_ptr<IfcConnectionGeometry>						m_connectionGeometry;
};

} // namespace IFCC

#endif // IFCC_SpaceBoundaryH
