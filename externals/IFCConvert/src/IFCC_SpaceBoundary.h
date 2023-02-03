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

	explicit SpaceBoundary(const SpaceBoundary&) = delete;

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

	/*! Copy content (exept id) from given space boundary.
		\param sb Other surface. All data except id and surface vector will be copied.
		\param surfaceIndex Index of surface from surface vector which will be the only surface of this one.
	*/
	void setFromSpaceBoundary(const SpaceBoundary& sb, size_t surfaceIndex);

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
	void setRelatingElementType(BuildingElementTypes type);

	/*! Get geometry from IFC basic geometry and fill the surface vector.
		\param unit_converter Unit converter from geometry converter
		\param spaceTransformation Coordinate transformation matrix
	*/
	bool fetchGeometryFromIFC(shared_ptr<UnitConverter>& unit_converter, const carve::math::Matrix& spaceTransformation,
							  std::vector<ConvertError>& errors);

	/*! Get geomtry from given surface.
		\param surface Surface for space boundary
	*/
	bool fetchGeometryFromBuildingElement(const Surface& surface);

	/*! In case of more than one surface after calling fetchGeometryFromIFC create additional space boundaries so each of them has only one surface.
		It uses the first surface in the vector for itself and clears the surface vector at the end.
		The returning vector will be empty in case of only one or no surfaces in surface vector.
	*/
	std::vector<std::shared_ptr<SpaceBoundary>> splitBySurfaces();

	/*! Return all surfaces of this space.*/
//	const std::vector<Surface>& surfaces() const {
//		return m_surfaces;
//	}

	/*! Return the surface of this space boundary.*/
	const Surface& surface() const 	{
		return m_surface;
	}

	/*! Return the surface of this space boundary.*/
	Surface& surface() 	{
		return m_surface;
	}

	/*! Return the surface of this space boundary.*/
	Surface surfaceWithSubsurfaces() const 	{
		Surface ts = m_surface;
		for(auto sub : m_containedOpeningSpaceBoundaries) {
			ts.addSubSurface(sub->surface());
		}
		return ts;
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

	/*! Return type of the related building element.*/
	BuildingElementTypes typeRelatedElement() const {
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

	/*! Return true if its a virtual space boundary (no real buildin element).*/
	bool isVirtual() const {
		return m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_VIRTUAL;
	}

	/*! Return true if the related building element could not be evaluated and a missing one is set.*/
	bool isMissing() const {
		return m_physicalOrVirtual == IfcPhysicalOrVirtualEnum::ENUM_PHYSICAL && m_elementEntityId == -1;
	}

	bool isInternal() const {
		return !isVirtual() && m_internalOrExternal == IfcInternalOrExternalEnum::ENUM_INTERNAL;
	}

	bool isExternalToGround() const {
		return !isVirtual() && m_internalOrExternal == IfcInternalOrExternalEnum::ENUM_EXTERNAL_EARTH;
	}

	bool isExternal() const {
		return !isVirtual() && m_internalOrExternal == IfcInternalOrExternalEnum::ENUM_EXTERNAL;
	}

	/*! Return the vector of all contained opening space boundaries.*/
	const std::vector<std::shared_ptr<SpaceBoundary> >& containedOpeningSpaceBoundaries() const;

	/*! Add a opening space boundarie to the vector of the contained space boundaries.*/
	void addContainedOpeningSpaceBoundaries(const std::shared_ptr<SpaceBoundary>& newContainedOpeningSpaceBoundaries);

	/*! Unique ID of the related building element.*/
	int															m_elementEntityId;

	/*! Unique ID of the related opening if space boundary is for an opening.
		Otherwise its -1;
	*/
	int															m_openingId;

	/*! Id of the component instance which uses this space boundary.*/
	int															m_componentInstanceId;


private:
	/*! Create the surface vector from given polylines and set surface types.*/
	void createSurfaceVect(const polyVector_t& polylines);

	std::string													m_guidRelatedElement;	///< GUID of the related building element
	std::string													m_guidRelatedSpace;	///< GUID of the related space
	std::string													m_guidCorrespondingBoundary;	///< GUID of the corresponding space boundary
	std::string													m_nameRelatedElement;	///< Name of the related building element
	std::string													m_nameRelatedSpace;	///< Name of the related space
	BuildingElementTypes										m_typeRelatedElement;	///< Object type of the related element
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
	/*! Temporary vector used from fetchGeometryFromIFC. Should be splitted later.*/
	std::vector<Surface>										m_surfaces;
	Surface														m_surface;				///< Surface of the space boundary
	std::shared_ptr<IfcConnectionGeometry>						m_connectionGeometry;	///< Geometry from IFC space boundary object
	/*! Type of the space boundary (A, B, inner) evaluated only in case of derived IFC type.*/
	SpaceBoundaryType											m_spaceBoundaryType;
	/*! Level of the space boundary (1st, 2nd, 3rd) evaluated only in case of derived IFC type.*/
	SpaceBoundaryLevelType										m_levelType;
	/*! Vector of space boundaries for openings which are containd in this space boundary (subsurfaces).*/
	std::vector<std::shared_ptr<SpaceBoundary>>					m_containedOpeningSpaceBoundaries;
};

} // namespace IFCC

#endif // IFCC_SpaceBoundaryH
