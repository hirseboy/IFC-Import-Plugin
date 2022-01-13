#ifndef IFCC_SpaceH
#define IFCC_SpaceH

#include <ifcpp/IFC4/include/IfcSpaceTypeEnum.h>
#include <ifcpp/IFC4/include/IfcSpace.h>

#include <IBKMK_Vector3D.h>

#include <tinyxml.h>

#include <VICUS_Room.h>

#include "IFCC_BuildingElement.h"
#include "IFCC_Helper.h"
#include "IFCC_Surface.h"
#include "IFCC_EntityBase.h"
#include "IFCC_SpaceBoundary.h"
#include "IFCC_Opening.h"

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
	bool set(std::shared_ptr<IfcSpace> ifcSpace);

	/*! Updates the space geometry from shape by calling fetchGeometry and transform.
		Set the transformation matrix.
		\param productShape Shape data of the space created from geometry converter.
	*/
	void update(std::shared_ptr<ProductShapeData> productShape);

	/*! Updates the type and content (surfaces) of the space boundaries. Also the connection to building elements will be set.
		If the IFC model doesn't contain space boundaries the function try to evaluate these from construction elements and openings.
		\param shapes Vector of element shaps with type
		\param unit_converter Unit converter from geometry converter
		\param constructionElements Vector of all construction elements (wall, roof, slab)
		\param openingElements Vector of all opening elements (window, door)
		\param openings Vector of openings with connections to opening elements and construction elements
		\return If false no space boundaries with connected building elements can be found (\sa m_spaceBoundaryErrors).
	*/
	bool updateSpaceBoundaries(const objectShapeTypeVector_t& shapes,
							   shared_ptr<UnitConverter>& unit_converter,
							   const std::vector<BuildingElement>& constructionElements,
							   const std::vector<BuildingElement>& openingElements,
							   const std::vector<Opening>& openings);

	/*! Return all surfaces of this space.*/
	const std::vector<Surface>& surfaces() const;

	/*! Return all space boundaries of this space.*/
	const std::vector<SpaceBoundary>& spaceBoundaries() const;

	/*! Write the space in vicus xml format including space boundaries.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Create a VICUS room object and return this.
		The returned object contains all transferable data.
	*/
	VICUS::Room getVicusObject(std::map<int,int>& idMap) const;

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
	IfcSpaceTypeEnum::IfcSpaceTypeEnumEnum		m_spaceType;
	/*! Some remarkes to the space. Can contain notes from space boundary evaluation.*/
	std::string									m_notes;
	/*! Contain error messages from space boundary evaluation function. Will be set from updateSpaceBoundaries if returns false.*/
	std::string									m_spaceBoundaryErrors;

private:

	/*! Transforms the space geometry by using transformation matrix from productShape.
		It transforms all coordinates from local system into global system.
	*/
	void transform(std::shared_ptr<ProductShapeData> productShape);

	/*! Get the geometry from the product shape.
		It fills the surface vector m_surfaces.
	*/
	void fetchGeometry(std::shared_ptr<ProductShapeData> productShape);

	/*! Is called from updateSpaceBoundaries in case space boundaries could be evaluated from IFC model.
		It get geometry for space boundaries and fill their surface vector.
		It set the the space boundary type and the id of the connected building element.
		\param shapes Vector of element shaps with type
		\param unit_converter Unit converter from geometry converter
		\param constructionElements Vector of all construction elements (wall, roof, slab)
		\param openingElements Vector of all opening elements (window, door)
	*/
	bool evaluateSpaceBoundaryTypes(const objectShapeTypeVector_t& shapes,
									 shared_ptr<UnitConverter>& unit_converter,
									 const std::vector<BuildingElement>& constructionElements,
									 const std::vector<BuildingElement>& openingElements);

	/*! Is called from updateSpaceBoundaries in case IFC model doesn't contain space boundaries.
		It try to evaluate space boundaries from construction elements and openings.
		\param constructionElements Vector of all construction elements (wall, roof, slab)
		\param openingElements Vector of all opening elements (window, door)
		\param openings Vector of all openings
	*/
	bool evaluateSpaceBoundaries(const std::vector<BuildingElement>& constructionElements,
								 const std::vector<BuildingElement>& openingElements,
								 const std::vector<Opening>& openings);

	/*! Try to find space boundaries for construction elements by test of surface properties (parallel, distance, intersections).
		Function will be called from evaluateSpaceBoundaries.
		\param constructionElements Vector for all construction elements with own surfaces
		\return Vector of evaluated space boundaries
	*/
	std::vector<SpaceBoundary> createSpaceBoundaries( const std::vector<BuildingElement>& constructionElements);

	/*! Try to find space boundaries for opening elements based on openings.
		\param spaceBoundaries Result vector for adding new space boundaries
		\param openingElements Vector of opening elements (window or door).
		\param openings Vector of all openings
	*/
	void createSpaceBoundariesForOpeningsFromOpenings(std::vector<SpaceBoundary>& spaceBoundaries,
													  const std::vector<BuildingElement>& openingElements,
													  const std::vector<Opening>& openings);

	/*! Update space surfaces from existing space boundaries.
		It fills the surface and the subsurface vectors.
		Then it evaluates which subsurface is part of a surface.
		If a surface is already intersected by a subsurface (has holes) the holes will be removed.
		At the end the subsurfaces will be added to the corresponding surfaces.
		The given building elements will be used in order to get intersection ranges from the element thicknesses.
		\param elems Vector of construction elements
	*/
	void updateSurfaces(const std::vector<BuildingElement>& elems);

	std::vector<SpaceBoundary>					m_spaceBoundaries;	///< Space boundaries of the space
	std::vector<Surface>						m_surfaces;			///< Surfaces of the space
	carve::math::Matrix							m_transformMatrix;	///< Matrix for geometry transformation from local to global coordinates
	std::vector<Surface>						m_surfacesOrg;		///< Original surfaces from the IFC model converted into global coordinates
};

} // namespace IFCC

#endif // IFCC_SpaceH
