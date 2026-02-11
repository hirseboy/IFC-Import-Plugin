#ifndef IFCC_BuildingH
#define IFCC_BuildingH

#include <ifcpp/IFC4X3/include/IfcSpatialStructureElement.h>
#include <ifcpp/IFC4X3/include/IfcBuildingStorey.h>

#include <tinyxml.h>

#include <VICUS_Building.h>

#include "IFCC_GeometryInputData.h"
#include "IFCC_EntityBase.h"
#include "IFCC_BuildingStorey.h"
#include "IFCC_BuildingElementsCollector.h"

namespace IFCC {

/*! Represents a building in a project.
	It can contain building storeys.
*/
class Building : public EntityBase
{
public:
	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit Building(int id);

	/*! Initialise building from a IFC building object.
		It set a name and fills the original storey vector.
	*/
	bool set(std::shared_ptr<IFC4X3::IfcSpatialStructureElement> ifcElement);

	/*! Create storey vector from original vector and given shapes.*/
	void fetchStoreys(const objectShapeGUIDMap_t& storeys, const objectShapeGUIDMap_t& spaces, bool onlyOne);

	/*! Update storeys and related spaces and space boundaries.
		\param elementShapes Product shape vector for all IfcElement based objects in project.
		\param spaceShapes Product shape vector for all IfcSpace based objects in project.
		\param unit_converter Unit converter is necessary for create geometries.
		\param constructionElements Vector for all construction elements (wall, slab, roof)
		\param openingElements Vector for all opening elements (window, door)
		\param openings Vector for all openings (based on IfcOpeningElement)
	*/
	bool updateStoreys(const objectShapeTypeVector_t& elementShapes,
					   const objectShapeGUIDMap_t& spaceShapes,
					   shared_ptr<UnitConverter>& unit_converter,
					   const BuildingElementsCollector& buildingElements,
					   std::vector<Opening>& openings,
					   bool useSpaceBoundaries,
					   std::vector<ConvertError>& errors,
					   const ConvertOptions& convertOptions);

	/*! Create a VICUS building object and return this.
		The returned object contains all transferable data.
	*/
	VICUS::Building getVicusObject(const ConvertOptions& options) const;

	/*! Write the building in vicus xml format including storeys.*/
	TiXmlElement * writeXML(TiXmlElement * parent, const ConvertOptions& convertOptions) const;

	/*! Return vector of building storeys.*/
	const std::vector<std::shared_ptr<BuildingStorey>>& storeys() const {
		return m_storeys;
	}

private:

	/*! Vector of building storeys.*/
	std::vector<std::shared_ptr<BuildingStorey>>	m_storeys;

	/*! Vector of original IFC building storey objects.*/
	std::vector<std::shared_ptr<IFC4X3::IfcBuildingStorey>>	m_storeysOriginal;

	/*! Vector of original IFC space objects.*/
	std::vector<std::shared_ptr<IFC4X3::IfcSpace>>	m_spacesOriginal;
};

} // namespace IFCC

#endif // IFCC_BuildingH
