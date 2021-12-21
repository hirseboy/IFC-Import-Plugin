#ifndef IFCC_SpaceBoundaryH
#define IFCC_SpaceBoundaryH

#include "IFCC_EntityBase.h"

#include <ifcpp/IFC4/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4/include/IfcPhysicalOrVirtualEnum.h>
#include <ifcpp/IFC4/include/IfcInternalOrExternalEnum.h>

#include <ifcpp/model/UnitConverter.h>

#include "IFCC_BuildingElement.h"

namespace IFCC {

/*! Class represents a space boundary.
	A space boundary is a planar surface which is part of a space and has connections to building elements.
*/
class SpaceBoundary : public IFCC::EntityBase
{
public:
	/*! Connected base construction type.*/
	enum ConstructionType {
		CT_OpeningElement,		///< Is a opening element (window, door)
		CT_ConstructionElement,	///< Is a construction element (wall, roof, slab)
		CT_Others,				///< Is a wether opening nor construction element (shading, rail, ramp ...)
		CT_Unknown				///< Type is not set or unknown
	};


	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit SpaceBoundary(int id);

	/*! Initialise space boundary from a IFC object.
		It set a name, guid, related element, type and basic geometry.
		\param ifcSpaceBoundary Original IFC space boundary element
	*/
	bool setFromIFC(std::shared_ptr<IfcRelSpaceBoundary> ifcSpaceBoundary);

	/*! Initialise space boundary from a building element object.
		It set a name, related element and type. Geometry is not set.
		\param ifcSpaceBoundary Original IFC space boundary element
	*/
	bool setFromBuildingElement(const std::string& name, const BuildingElement& elem);

	/*! Set connection base type from building element type.*/
	void setRelatingElementType(ObjectTypes type);

	/*! Get geometry from IFC basic geometry and fill the surface vector.
		\param unit_converter Unit converter from geometry converter
		\param spaceTransformation Coordinate transformation matrix
	*/
	bool fetchGeometryFromIFC(shared_ptr<UnitConverter>& unit_converter, const carve::math::Matrix& spaceTransformation);

	/*! Get geomtry from given surface.
		\param surface Surface for space boundary
	*/
	bool fetchGeometryFromBuildingElement(const Surface& surface);

	/*! Return all surfaces of this space.*/
	const std::vector<Surface>& surfaces() const {
		return m_surfaces;
	}

	/*! Return GUID of the related building element.*/
	std::string guidRelatedElement() const {
		return m_guidRelatedElement;
	}

	/*! Return the name of the related building element.*/
	std::string nameRelatedElement() const {
		return m_nameRelatedElement;
	}

	/*! Return true if the space boundary is connected to a construction element.*/
	bool isConstructionElement() const {
		return m_type == CT_ConstructionElement;
	}

	/*! Return true if the space boundary is connected to a opening element.*/
	bool isOpeningElement() const {
		return m_type == CT_OpeningElement;
	}

	/*! Unique ID of the related building element.*/
	int															m_elementEntityId;

private:
	/*! Create the surface vector from given polylines and set surface types.*/
	void createSurfaceVect(const polyVector_t& polylines);

	std::string													m_guidRelatedElement;	///< GUID of the related building element
	std::string													m_nameRelatedElement;	///< Name of the related building element
	ObjectTypes													m_typeRelatedElement;	///< Object type of the related element
	/*! Defines if the space boundary is a physical or a virtual boundary.
		Possible types:
		\li ENUM_PHYSICAL - a physical object (wall, slab, door ...)
		\li ENUM_VIRTUAL - a virtual object only for dividing a space
		\li ENUM_NOTDEFINED - not known
	*/
	IfcPhysicalOrVirtualEnum::IfcPhysicalOrVirtualEnumEnum		m_physicalOrVirtual;
	/*! Defines if the space boundary is a internal or external boundary.
		Possible types:
		\li ENUM_INTERNAL - internal boundary
		\li ENUM_EXTERNAL - external boundary to air
		\li ENUM_EXTERNAL_EARTH - external boundary to ground
		\li ENUM_EXTERNAL_WATER - external boundary to water
		\li ENUM_EXTERNAL_FIRE - boundary to fire
		\li ENUM_NOTDEFINED - not defined
		It is used to define the surface position type (\sa Surface::setSurfaceType).
	*/
	IfcInternalOrExternalEnum::IfcInternalOrExternalEnumEnum	m_internalOrExternal;
	ConstructionType											m_type;					///< Type of connected construction
	std::vector<Surface>										m_surfaces;				///< Surfaces of the space boundary
	std::shared_ptr<IfcConnectionGeometry>						m_connectionGeometry;	///< Geometry from IFC space boundary object
};

} // namespace IFCC

#endif // IFCC_SpaceBoundaryH
