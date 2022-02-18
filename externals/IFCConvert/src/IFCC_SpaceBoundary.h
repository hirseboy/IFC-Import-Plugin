#ifndef IFCC_SpaceBoundaryH
#define IFCC_SpaceBoundaryH

#include "IFCC_EntityBase.h"

#include <ifcpp/IFC4/include/IfcRelSpaceBoundary.h>
#include <ifcpp/IFC4/include/IfcPhysicalOrVirtualEnum.h>
#include <ifcpp/IFC4/include/IfcInternalOrExternalEnum.h>

#include <ifcpp/model/UnitConverter.h>

#include "IFCC_BuildingElement.h"

namespace IFCC {

class Space;

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

	/*! Defines the internal space boundary type.*/
	enum SpaceBoundaryType {
		SBT_A,			///< Type 2a - space boundary is connected to construction which have a space boundary at other side
		SBT_B,			///< Type 2b - space boundary is connected to construction without other side (e.g. inner wall junction)
		SBT_Inner,		///< Inner space boundary which is connected to a opening element like a door. Should have a parent space boundary type 2a
		SBT_Unknown,	///< Type is not set or unknown
	};

	enum SpaceBoundaryLevelType {
		SBLT_NoLevel,	///< Space boundary has no level
		SBLT_1stLevel,	///< First level - only plain inner surfaces
		SBLT_2ndLevel,	///< Second level - divided into type 2a and 2b for plain walls and junctions
		SBLT_3rdLevel	///< Currently not used in IFC
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
		It set a name, related element, related space and type. Geometry is not set.
		\param name Name for space boundary
		\param elem Building element which is related to.
		\param space Space which is related to.
	*/
	bool setFromBuildingElement(const std::string& name, const std::shared_ptr<BuildingElement>& elem,
								const IFCC::Space& space);

	/*! Initialise space boundary which doesn't have a connection to a buiding element.
		It set a name, related space and type. Geometry is not set.
		\param name Name for space boundary.
		\param space Space which is related to.
	*/
	void setForMissingElement(const std::string& name, const Space& space);

	/*! Initialise space boundary which doesn't have a connection to a buiding element.
		It set a name, related space and type. Geometry is not set.
		\param name Name for space boundary.
		\param space Space which is related to.
	*/
	void setForVirtualElement(const std::string& name, const Space& space);

	/*! Set connection base type from building element type.*/
	void setRelatingElementType(ObjectTypes type);

	/*! Get geometry from IFC basic geometry and fill the surface vector.
		\param unit_converter Unit converter from geometry converter
		\param spaceTransformation Coordinate transformation matrix
	*/
	bool fetchGeometryFromIFC(shared_ptr<UnitConverter>& unit_converter, const carve::math::Matrix& spaceTransformation, std::string& errmsg);

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

	/*! Return GUID of the related space.*/
	std::string guidRelatedSpace() const {
		return m_guidRelatedSpace;
	}

	/*! Return the name of the related space.*/
	std::string nameRelatedSpace() const {
		return m_nameRelatedSpace;
	}

	ObjectTypes typeRelatedElement() const {
		return m_typeRelatedElement;
	}

	/*! Return true if the space boundary is connected to a construction element.*/
	bool isConstructionElement() const {
		return m_type == CT_ConstructionElement;
	}

	/*! Return true if the space boundary is connected to a opening element.*/
	bool isOpeningElement() const {
		return m_type == CT_OpeningElement;
	}

	bool isVirtual() const {
		return m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL;
	}

	bool isMissing() const {
		return m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_PHYSICAL && m_elementEntityId == -1;
	}

	/*! Unique ID of the related building element.*/
	int															m_elementEntityId;

private:
	/*! Create the surface vector from given polylines and set surface types.*/
	void createSurfaceVect(const polyVector_t& polylines);

	std::string													m_guidRelatedElement;	///< GUID of the related building element
	std::string													m_guidRelatedSpace;	///< GUID of the related space
	std::string													m_guidCorrespondingBoundary;	///< GUID of the corresponding space boundary
	std::string													m_nameRelatedElement;	///< Name of the related building element
	std::string													m_nameRelatedSpace;	///< Name of the related space
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
	SpaceBoundaryType											m_spaceBoundaryType;
	SpaceBoundaryLevelType										m_levelType;
};

} // namespace IFCC

#endif // IFCC_SpaceBoundaryH
