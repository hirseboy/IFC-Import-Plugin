#ifndef IFCC_BuildingStoreyH
#define IFCC_BuildingStoreyH

#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/IFC4/include/IfcSpace.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>

#include <tinyxml.h>

//#include <VICUS_BuildingLevel.h>

#include "IFCC_EntityBase.h"
#include "IFCC_Space.h"
#include "IFCC_Types.h"
#include "IFCC_BuildingElementsCollector.h"

namespace IFCC {

/*! Represents a building storey in a project.
	It can contain spaces.
*/
class BuildingStorey : public EntityBase
{
public:
	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit BuildingStorey(int id);

	/*! Initialise storey from a IFC storey object.
		It set a name and fills the original storey vector.
		\param ifcElement IFC object for a building storey. It uses the base class of IfcBuildingStorey.
	*/
	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement);

	/*! Set a storeys which contains all spaces in project.*/
	bool set(const objectShapeGUIDMap_t& spaces);

	/*! Set a storeys which contains all spaces contained in a building.*/
	bool set(const std::vector<std::shared_ptr<IfcSpace>>& spaces);

	/*! Create spaces vector from original vector and given shapes.
		\param shapes Product shape for all spaces in project.
		\param unit_converter Unit converter is necessary for create geometries.
	*/
	void fetchSpaces(const objectShapeGUIDMap_t& shapes,
					 shared_ptr<UnitConverter>& unit_converter);

	/*! Update spaces and related space boundaries.
		\param shapes Product shape vector for all IfcElement based objects in project.
		\param unit_converter Unit converter is necessary for create geometries.
		\param constructionElements Vector for all construction elements (wall, slab, roof)
		\param openingElements Vector for all opening elements (window, door)
		\param openings Vector for all openings (based on IfcOpeningElement)
		\return If false no space boundaries with connected building elements can be found (\sa m_spaceBoundaryErrors).
	*/
	void updateSpaces(const objectShapeTypeVector_t& shapes,
					  shared_ptr<UnitConverter>& unit_converter,
					  const BuildingElementsCollector& buildingElements,
					  std::vector<Opening>& openings,
					  bool useSpaceBoundaries,
					  std::vector<ConvertError>& errors);

//	/*! Update spaces and related space boundaries.
//		\param constructionElements Vector for all construction elements (wall, slab, roof)
//		\param openingElements Vector for all opening elements (window, door)
//		\param openings Vector for all openings (based on IfcOpeningElement)
//	*/
//	void updateSpaceConnections(BuildingElementsCollector& buildingElements, std::vector<Opening>& openings);

	/*! Write the storey in vicus xml format including spaces.*/
	TiXmlElement * writeXML(TiXmlElement * parent, bool flipPolygons) const;

//	/*! Create a VICUS building level object and return this.
//		The returned object contains all transferable data.
//	*/
//	VICUS::BuildingLevel getVicusObject(std::map<int,int>& idMap, int& nextid) const;

	/*! Vector of spaces in the storey.*/
	const std::vector<std::shared_ptr<Space>>& spaces() const {
		return m_spaces;
	}

private:

	/*! Vector of spaces in the storey.*/
	std::vector<std::shared_ptr<Space>>						m_spaces;
	/*! Vector of original IFC space objects.*/
	std::vector<std::shared_ptr<IfcSpace>>	m_spacesOriginal;
};

} // namespace IFCC

#endif // IFCC_BuildingStoreyH
