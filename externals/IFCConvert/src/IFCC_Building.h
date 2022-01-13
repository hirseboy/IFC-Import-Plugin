#ifndef IFCC_BuildingH
#define IFCC_BuildingH

#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>
#include <ifcpp/IFC4/include/IfcBuildingStorey.h>

#include <tinyxml.h>

#include <VICUS_Building.h>

#include "IFCC_EntityBase.h"
#include "IFCC_BuildingStorey.h"

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
	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement);

	/*! Create storey vector from original vector and given shapes.*/
	void fetchStoreys(const std::map<std::string,shared_ptr<ProductShapeData>>& openings);

	/*! Update storeys and related spaces and space boundaries.
		\param elementShapes Product shape vector for all IfcElement based objects in project.
		\param spaceShapes Product shape vector for all IfcSpace based objects in project.
		\param unit_converter Unit converter is necessary for create geometries.
		\param constructionElements Vector for all construction elements (wall, slab, roof)
		\param openingElements Vector for all opening elements (window, door)
		\param openings Vector for all openings (based on IfcOpeningElement)
	*/
	void updateStoreys(const objectShapeTypeVector_t& elementShapes,
					   const objectShapeGUIDMap_t& spaceShapes,
					  shared_ptr<UnitConverter>& unit_converter,
					  const std::vector<BuildingElement>& constructionElemnts,
					  const std::vector<BuildingElement>& openingElements,
					  const std::vector<Opening>& openings);

	/*! Write the building in vicus xml format including storeys.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Create a VICUS building object and return this.
		The returned object contains all transferable data.
		\param idMap Used for mapping current ids to VICUS ids.
	*/
	VICUS::Building getVicusObject(std::map<int,int>& idMap) const;

	/*! Return vector of building storeys.*/
	const std::vector<BuildingStorey>& storeys() const {
		return m_storeys;
	}

private:

	/*! Vector of building storeys.*/
	std::vector<BuildingStorey>						m_storeys;
	/*! Vector of original IFC building storey objects.*/
	std::vector<std::shared_ptr<IfcBuildingStorey>>	m_storeysOriginal;
};

} // namespace IFCC

#endif // IFCC_BuildingH
