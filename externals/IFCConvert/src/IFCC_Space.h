#ifndef IFCC_SpaceH
#define IFCC_SpaceH

#include <ifcpp/IFC4X3/include/IfcSpaceTypeEnum.h>
#include <ifcpp/IFC4X3/include/IfcSpace.h>

#include <IBKMK_Vector3D.h>

#include <tinyxml.h>

#include <carve/poly.hpp>

#include "IFCC_BuildingElement.h"
#include "IFCC_Helper.h"
#include "IFCC_Surface.h"
#include "IFCC_EntityBase.h"
#include "IFCC_SpaceBoundary.h"
#include "IFCC_Opening.h"
#include "IFCC_BuildingElementsCollector.h"

namespace IFCC {

/*! Class represents a space (room) in a building.
	It contains also all space boundaries as connection from space surface to building or opening elements.
*/
class Space : public EntityBase
{
public:

	/*! Type of matching algorithm for openings.*/
	enum OpeningMatchingType {
		OMT_SamePoints,					///< Opening and space boundary has identical points
		OMT_WallThicknessIntersection,	///< A intersction is found inside the distance of the corresponding construction thickness
		OMT_NoMatching
	};

	/*! Contains result of the opening matching functions.*/
	struct OpeningMatching {
		/*! Default constructor. Create a non-valid object.*/
		OpeningMatching() :
			m_type(OMT_NoMatching),
			m_surfaceIndex(-1),
			m_subSurfaceIndex(-1)
		{}

		/*! Standard constructor.
			\param type Type of used matching algorithm
			\param si Index of found surface
			\param subi Index of matching subsurface
		*/
		OpeningMatching(OpeningMatchingType type, size_t si, size_t subi) :
			m_type(type),
			m_surfaceIndex(si),
			m_subSurfaceIndex(subi)
		{}

		OpeningMatchingType m_type;				///< Type of used matching algorithm
		size_t				m_surfaceIndex;		///< Index of found surface
		size_t				m_subSurfaceIndex;	///< Index of matching subsurface
	};

	/*! Contains result of space boundary matching functions.*/
	struct OpeningConstructionMatching {
		/*! Default constructor. Create a non-valid object.*/
		OpeningConstructionMatching() :
			m_constructionSurfaceIndex(-1),
			m_openingIndex(-1),
			m_openingSurfaceIndex(-1)
		{}

		/*! Standard constructor.
			\param constructionSurfaceIndex Index of surface in current construction
			\param m_openingIndex Index of opening
			\param m_openingSurfaceIndex Index of surface in the opening
		*/
		OpeningConstructionMatching(int constructionSurfaceIndex, int spaceBoundaryIndex, int spaceBoundarySurfaceIndex) :
			m_constructionSurfaceIndex(constructionSurfaceIndex),
			m_openingIndex(spaceBoundaryIndex),
			m_openingSurfaceIndex(spaceBoundarySurfaceIndex)
		{}

		/*! Return true if all indices are valid.*/
		bool isValid() const {
			if(m_constructionSurfaceIndex < 0)
				return false;
			if(m_openingIndex < 0)
				return false;
			if(m_openingSurfaceIndex < 0)
				return false;
			return true;
		}

		int m_constructionSurfaceIndex;		///< Index of surface in current construction
		int m_openingIndex;			///< Index of opening
		int m_openingSurfaceIndex;	///< Index of surface in opening
	};

	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit Space(int id);

	/*! Initialise space from a IFC building element object.
		It set a name, long name and space type..
		Space boundaries will be created if the IFC data include some.
		\param ifcSpace Original IFC space element
	*/
	bool set(std::shared_ptr<IFC4X3::IfcSpace> ifcSpace);

	/*! Updates the space geometry from shape by calling fetchGeometry and transform.
		Set the transformation matrix.
		\param productShape Shape data of the space created from geometry converter.
	*/
	void update(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors);

	/*! Updates the type and content (surfaces) of the space boundaries. Also the connection to building elements will be set.
		If the IFC model doesn't contain space boundaries the function try to evaluate these from construction elements and openings.
		\param shapes Vector of element shaps with type
		\param unit_converter Unit converter from geometry converter
		\param constructionElements Vector of all construction elements (wall, roof, slab)
		\param openingElements Vector of all opening elements (window, door)
		\param openings Vector of openings with connections to opening elements and construction elements
		\return If false no space boundaries with connected building elements can be found (\sa m_spaceBoundaryErrors).
		In case of evaluated space boundaries for openings a connection will be set in the corresponding opening.
	*/
	bool updateSpaceBoundaries(const objectShapeTypeVector_t& shapes,
							   shared_ptr<UnitConverter>& unit_converter,
							   const BuildingElementsCollector& buildingElements,
							   std::vector<Opening>& openings,
							   bool useSpaceBoundaries, std::vector<ConvertError>& errors);

	/*! Return all space boundaries of this space.*/
	const std::vector<std::shared_ptr<SpaceBoundary>>& spaceBoundaries() const;

	/*! If space boundaries with the same surface exist one of whem will be removed.
	*/
	void removeDublicatedSpaceBoundaries();

	/*! Check if subsurfaces are used more than once.
		\return vector of space boundary ids which are used more than once as subsurface
	*/
	std::vector<int> checkUniqueSubSurfaces() const;

	/*! Check if some space boundaries have the same surface.
		\param equalSBs vector of ids of the equal space boundaries
	*/
	void checkForEqualSpaceBoundaries(std::vector<std::pair<int,int>>& equalSBs) const;

	/*! Check if the current space is intersected to the other one.*/
	bool isIntersected(const Space& other) const;

	/*! Write the space in vicus xml format including space boundaries.
		\param parent Parent xml node
		\param positiveRotation If true all polygons must have poitive rotation type othewise a negative one.
	*/
	TiXmlElement * writeXML(TiXmlElement * parent, bool positiveRotation) const;

	/*! Return true if the space contains a space boundary with the given GUID.*/
	bool hasSpaceBoundary(const std::string& guid) const;

	/*! Return true if the two spaces share one space boundary.*/
	bool shareSameSpaceBoundary(const Space& space) const;

	std::string									m_longName;			///< More detailed name of the space
	/*! IFC space type. It can be:
		\li ENUM_SPACE - Any space not falling into another category.
		\li ENUM_PARKING - A space dedication for use as a parking spot
		\li ENUM_GFA - Gross Floor Area - a specific kind of space for each building story that includes all net area and construction area
		\li ENUM_INTERNAL - not defined yet
		\li ENUM_EXTERNAL - not defined yet
		\li ENUM_USERDEFINED - user defined space type
		\li ENUM_NOTDEFINED - undefined space type
	*/
	IFC4X3::IfcSpaceTypeEnum::IfcSpaceTypeEnumEnum		m_spaceType;
	/*! Some remarkes to the space. Can contain notes from space boundary evaluation.*/
	std::string									m_notes;

private:

	/*! Transforms the space geometry by using transformation matrix from productShape.
		It transforms all coordinates from local system into global system.
	*/
	void transform(std::shared_ptr<ProductShapeData> productShape);

	/*! Get the geometry from the product shape.
		It fills the surface vector m_surfaces.
	*/
	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape, std::vector<ConvertError>& errors);

	/*! Is called from evaluateSpaceBoundaryFromIFC.
		It set the the space boundary type and the id of the connected building element.
		\param shapes Vector of element shaps with type
		\param buildingElements Vector of all building elements (wall, roof, slab, window, door etc.)
	*/
	void evaluateSpaceBoundaryTypes(const objectShapeTypeVector_t& shapes,
									 const BuildingElementsCollector& buildingElements);

	/*! Is called from evaluateSpaceBoundaryFromIFC.
		It get geometry for space boundaries and fill their temporary surface vector.
		\param unit_converter Unit converter from geometry converter
		\param errors Vector of all conversion errors which occures here
	*/
	bool evaluateSpaceBoundaryGeometry(shared_ptr<UnitConverter>& unit_converter,
									   std::vector<ConvertError>& errors);

	/*! Is called from updateSpaceBoundaries in case space boundaries could be evaluated from IFC model.
		It set the the space boundary type and the id of the connected building element (evaluateSpaceBoundaryTypes).
		It get geometry for space boundaries and fill their temporary surface vector (evaluateSpaceBoundaryGeometry).
		It splits all space boundaries which have more than one surface and adds the result to the space vector.
		It set the connections between construction space boundaries and opening space boundaries.
		\param shapes Vector of element shaps with type
		\param buildingElements Vector of all building elements (wall, roof, slab, window, door etc.)
		\param unit_converter Unit converter from geometry converter
		\param errors Vector of all conversion errors which occures here
	*/
	bool evaluateSpaceBoundaryFromIFC(const objectShapeTypeVector_t& shapes,
									  const BuildingElementsCollector& buildingElements,
									  shared_ptr<UnitConverter>& unit_converter,
									  std::vector<ConvertError>& errors);

	/*! Is called from updateSpaceBoundaries in case IFC model doesn't contain space boundaries.
		It try to evaluate space boundaries from construction elements and openings.
		\param buildingElements Vector of all building elements (wall, roof, slab, window, door etc.)
		\param openings Vector of all openings
		\param errors Vector of all conversion errors which occures here
	*/
	bool evaluateSpaceBoundariesFromConstruction(const BuildingElementsCollector& buildingElements,
												 std::vector<Opening>& openings, std::vector<ConvertError>& errors);

	/*! Try to find space boundaries for construction elements by test of surface properties (parallel, distance, intersections).
		Function will be called from evaluateSpaceBoundaries.
		\param constructionElements Vector for all construction elements with own surfaces
		\return Vector of evaluated space boundaries
	*/
	std::vector<std::shared_ptr<SpaceBoundary>> createSpaceBoundaries( const BuildingElementsCollector& buildingElements, std::vector<ConvertError>& errors);

	/*! Try to find space boundaries for opening elements based on openings.
		\param spaceBoundaries Result vector for adding new space boundaries
		\param openingElements Vector of opening elements (window or door).
		\param openings Vector of all openings
	*/
	void createSpaceBoundariesForOpeningsFromOpenings(std::vector<std::shared_ptr<SpaceBoundary>>& spaceBoundaries,
													  const BuildingElementsCollector& buildingElements,
													  const std::vector<Opening>& openings, std::vector<ConvertError>& errors);

	/*! Create opening space boundaries by matching openings to existing space boundaries.*/
	void createSpaceBoundariesForOpeningsFromSpaceBoundaries(std::vector<std::shared_ptr<SpaceBoundary>>& spaceBoundaries,
															 const BuildingElementsCollector& buildingElements,
															 std::vector<Opening>& openings, std::vector<ConvertError>& errors);

	std::vector<std::shared_ptr<SpaceBoundary>>				m_spaceBoundaries;	///< Space boundaries of the space
	carve::math::Matrix										m_transformMatrix;	///< Matrix for geometry transformation from local to global coordinates
	std::vector<Surface>									m_surfacesOrg;		///< Original surfaces from the IFC model converted into global coordinates
	meshVector_t											m_meshSets;
	std::vector<std::string>								m_spaceBoundaryGUIDs;
};

} // namespace IFCC

#endif // IFCC_SpaceH
