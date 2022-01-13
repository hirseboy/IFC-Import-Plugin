#ifndef IFCC_BuildingStoreyH
#define IFCC_BuildingStoreyH

#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/IFC4/include/IfcSpace.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>

#include <tinyxml.h>

#include <VICUS_BuildingLevel.h>

#include "IFCC_EntityBase.h"
#include "IFCC_Space.h"

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
	*/
	void updateSpaces(const objectShapeTypeVector_t& shapes,
					  shared_ptr<UnitConverter>& unit_converter,
					  const std::vector<BuildingElement>& constructionElemnts,
					  const std::vector<BuildingElement>& openingElements,
					  const std::vector<Opening>& openings);

	/*! Write the storey in vicus xml format including spaces.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Create a VICUS building level object and return this.
		The returned object contains all transferable data.
	*/
	VICUS::BuildingLevel getVicusObject(std::map<int,int>& idMap) const;

	/*! Vector of spaces in the storey.*/
	const std::vector<Space>& spaces() const {
		return m_spaces;
	}

private:

	/*! Vector of spaces in the storey.*/
	std::vector<Space>						m_spaces;
	/*! Vector of original IFC space objects.*/
	std::vector<std::shared_ptr<IfcSpace>>	m_spacesOriginal;
};

} // namespace IFCC

#endif // IFCC_BuildingStoreyH
