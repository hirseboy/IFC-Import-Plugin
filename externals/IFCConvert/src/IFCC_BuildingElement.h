#ifndef IFCC_BuildingElementH
#define IFCC_BuildingElementH


#include <ifcpp/IFC4/include/IfcElement.h>
#include <ifcpp/IFC4/include/IfcWall.h>
#include <ifcpp/IFC4/include/IfcOpeningElement.h>
#include <ifcpp/IFC4/include/IfcWindow.h>
#include <ifcpp/IFC4/include/IfcWindowTypeEnum.h>
#include <ifcpp/IFC4/include/IfcDoor.h>
#include <ifcpp/IFC4/include/IfcDoorTypeEnum.h>
#include <ifcpp/IFC4/include/IfcDoorTypeOperationEnum.h>
#include <ifcpp/IFC4/include/IfcWindowTypePartitioningEnum.h>
#include <ifcpp/IFC4/include/IfcWallTypeEnum.h>
#include <ifcpp/IFC4/include/IfcWallType.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>

#include "IFCC_Types.h"
#include "IFCC_Surface.h"
#include "IFCC_Opening.h"
#include "IFCC_EntityBase.h"
#include "IFCC_Property.h"

namespace IFCC {

/*! Class represents a building element. This can be each element in a building.
	The main elements are construction elements (wall, slab ...) and opening elements (window, door, ...).
*/
class BuildingElement : public EntityBase
{
public:
	/*! Internal structure which holds properties for opening elements like a window or a door.*/
	struct OpeningProperties {
		bool						m_isWindow = false;		///< Is it a window
		bool						m_isDoor = false;		///< is it a door
		int							m_id = -1;				///< internal id
		/*! Vector of ids for construction which contains the current opening.*/
		std::vector<int>			m_usedInConstructionIds;
		/*! Thicknesses of all constructions which contains the current opening.
			Should have the same size as m_usedInConstructionIds.
		*/
		std::vector<double>			m_constructionThicknesses;
		std::string					m_typeName;					///< Name of the opening type
		// window properties
		double						m_windowHeight = 0;
		double						m_windowWidth = 0;
		std::string					m_windowUserDefinedPartitionType;	///< Type name of a user defined window.
		std::vector<std::string>	m_windowConstructionTypes;			///< Construction type names for the window
		/*! Window type enum. Following is possible:
			\li ENUM_WINDOW
			\li ENUM_SKYLIGHT
			\li ENUM_LIGHTDOME
			\li ENUM_USERDEFINED
			\li ENUM_NOTDEFINED
		*/
		IfcWindowTypeEnum::IfcWindowTypeEnumEnum m_windowType = IfcWindowTypeEnum::ENUM_NOTDEFINED;
		/*! Window partition type enum. Following is possible:
			\li ENUM_SINGLE_PANEL,
			\li ENUM_DOUBLE_PANEL_VERTICAL,
			\li ENUM_DOUBLE_PANEL_HORIZONTAL,
			\li ENUM_TRIPLE_PANEL_VERTICAL,
			\li ENUM_TRIPLE_PANEL_BOTTOM,
			\li ENUM_TRIPLE_PANEL_TOP,
			\li ENUM_TRIPLE_PANEL_LEFT,
			\li ENUM_TRIPLE_PANEL_RIGHT,
			\li ENUM_TRIPLE_PANEL_HORIZONTAL,
			\li ENUM_USERDEFINED,
			\li ENUM_NOTDEFINED
		*/
		IfcWindowTypePartitioningEnum::IfcWindowTypePartitioningEnumEnum m_windowPartitionType =
				IfcWindowTypePartitioningEnum::ENUM_NOTDEFINED;
		// Door properties
		double						m_doorHeight;
		double						m_doorWidth;
		/*! Door type enum. Following is possible:
			\li ENUM_DOOR,
			\li ENUM_GATE,
			\li ENUM_TRAPDOOR,
			\li ENUM_USERDEFINED,
			\li ENUM_NOTDEFINED
		*/
		IfcDoorTypeEnum::IfcDoorTypeEnumEnum	m_doorType;
		/*! Door operation type enum. Following is possible:
			\li ENUM_SINGLE_SWING_LEFT,
			\li ENUM_SINGLE_SWING_RIGHT,
			\li ENUM_DOUBLE_DOOR_SINGLE_SWING,
			\li ENUM_DOUBLE_DOOR_SINGLE_SWING_OPPOSITE_LEFT,
			\li ENUM_DOUBLE_DOOR_SINGLE_SWING_OPPOSITE_RIGHT,
			\li ENUM_DOUBLE_SWING_LEFT,
			\li ENUM_DOUBLE_SWING_RIGHT,
			\li ENUM_DOUBLE_DOOR_DOUBLE_SWING,
			\li ENUM_SLIDING_TO_LEFT,
			\li ENUM_SLIDING_TO_RIGHT,
			\li ENUM_DOUBLE_DOOR_SLIDING,
			\li ENUM_FOLDING_TO_LEFT,
			\li ENUM_FOLDING_TO_RIGHT,
			\li ENUM_DOUBLE_DOOR_FOLDING,
			\li ENUM_REVOLVING,
			\li ENUM_ROLLINGUP,
			\li ENUM_SWING_FIXED_LEFT,
			\li ENUM_SWING_FIXED_RIGHT,
			\li ENUM_USERDEFINED,
			\li ENUM_NOTDEFINED
		*/
		IfcDoorTypeOperationEnum::IfcDoorTypeOperationEnumEnum	m_doorOperationType;
		/*! Name of the user defined operation type.*/
		std::string					m_doorUserDefinedOperationType;
	};

	struct WallProperties {
		void update(std::shared_ptr<IfcWall>& ifcWall);

		IfcWallTypeEnum::IfcWallTypeEnumEnum	m_wallType = IfcWallTypeEnum::ENUM_USERDEFINED;
	};

	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit BuildingElement(int id);

	/*! Initialise building element from a IFC building element object.
		It set a name and fills the original storey vector.
		\param ifcElement Original IFC building element
		\param type type from ObjectTypes evaluated from concrete IFC class type
	*/
	bool set(std::shared_ptr<IfcElement> ifcElement, BuildingElementTypes type);

	/*! get and transform geometry and set first opening properties.
		\param Shape data of a building element.
	*/
	void update(std::shared_ptr<ProductShapeData> productShape, std::vector<Opening>& openings);

	/*! Only for opening constructions.
		Set also the ids for construction which contains opening elements
		Must be called after update().
		\param openings All openings in the project
	*/
	void setContainingElements(const std::vector<Opening>& openings);

	/*! Only for opening constructions.
		Fills vector of thicknesses of containing constructions.
		Must be called after update() and setContainingElements().
		\param elements All construction building elements in the project
	*/
	void setContainedConstructionThickesses(const std::vector<std::shared_ptr<BuildingElement>>& elements);

	/*! Return the internal surface vector.*/
	const std::vector<Surface>& surfaces() const;

	/*! Return the possible thickness of a building object.
		It evaluates the thicknes by search for the minimum distance betwee two parallel surfaces of this building element.
		This works only correctly for construction elements.
	*/
	double	thickness() const;

	/*! It returns the area of the bounding rect if this element is a opening element.
		If its a construction element it will return 0.
	*/
	double openingArea() const;

	/*! Returns true if the element can be used as subsurface component (opening element).*/
	bool isSubSurfaceComponent() const {
		return m_subSurfaceComponent;
	}

	/*! Returns true if the element can be used as surface component (construction element).*/
	bool isSurfaceComponent() const {
		return m_surfaceComponent;
	}

	/*! Returns true if the element is wether surface nor subsurface component (virtual element, furniture, building system, ...).*/
	bool isOtherComponent() const {
		return !m_surfaceComponent && !m_subSurfaceComponent;
	}

	/*! Return type of the building element.*/
	BuildingElementTypes type() const {
		return m_type;
	}

	/*! Vector of thickness and name for the layers of this element.*/
	std::vector<std::pair<double,std::string>>							m_materialLayers;
	/*! Vector of material property maps with the same order as the layers vector.*/
	std::vector<std::map<std::string,std::map<std::string,Property>>>	m_materialPropertyMap;
	/*! This building element is used from the openings given by their ids.
		That means it must be a opening element (window or door).*/
	std::vector<int>													m_usedFromOpenings;
	/*! Vector of opening ids which are contained in this building element.
		Its only filled if this is a construction element.*/
	std::vector<int>													m_containedOpenings;
	/*! Id of the element construction. It is used to generate component instances.*/
	int																	m_constructionId;
	/*! Contains additional properties only for opening elements.*/
	OpeningProperties													m_openingProperties;
	/*! Contains additional properties only for walls.*/
	WallProperties														m_wallProperties;
	/*! Vector of surface pairs (given by index) which are parallel.
		This vector is used in order to evaluate element thickness.
	*/
	std::vector<std::pair<int,int>>										m_parallelSurfaces;

	/*! Map store the surface indices which are connected to a space given by ID.
		Map key is id of space.
		First value is index if space surface.
		Second value is index of opening surface.
	*/
	std::map<int,std::vector<std::pair<size_t,size_t>>>	m_spaceSurfaceConnection;

private:
	/*! Fille the surface pair vector.*/
	void findSurfacePairs();

	/*! Transforms the element geometry by using transformation matrix from productShape.
		It transforms all coordinates from local system into global system.
	*/
	void transform(std::shared_ptr<ProductShapeData> productShape);

	/*! Get the geometry from the product shape.
		It fills the surface vector m_surfaces.
	*/
	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	/*! Find connections from this building element to openings.
		It uses the IFC connection vectors m_isUsedFromOpeningsOriginal and m_containedOpeningsOriginal.
		It fills the vectors m_usedFromOpenings and m_containedOpenings.
		Furthermore the connection ids in the opening objects will be set.
	*/
	void fetchOpenings(std::vector<Opening>& openings);

	void setThermalTransmittance();

	BuildingElementTypes													m_type;					///< Type of the building element
	bool														m_surfaceComponent;		///< If true its a construction element usable as surface
	bool														m_subSurfaceComponent;	///< If true its a opening element usable as subsurface.
	std::vector<Surface>										m_surfaces;				///< Vector of all surfaces
	/*! Vector of IFC opening object which uses this element.*/
	std::vector<std::shared_ptr<IfcOpeningElement>>				m_isUsedFromOpeningsOriginal;
	/*! Vector of IFC opening objects which are contained in this building element.*/
	std::vector<std::shared_ptr<IfcFeatureElementSubtraction>>	m_containedOpeningsOriginal;
	double														m_thermalTransmittance;
	std::map<std::string,std::map<std::string,Property>>		m_propertyMap;
};

} // namespace IFCC

#endif // IFCC_BuildingElementH
