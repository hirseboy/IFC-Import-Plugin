#ifndef IFCC_BuildingH
#define IFCC_BuildingH

#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>
#include <ifcpp/IFC4/include/IfcBuildingStorey.h>

#include <tinyxml.h>

#include "IFCC_EntityBase.h"
#include "IFCC_BuildingStorey.h"

namespace IFCC {

class Building : public EntityBase
{
public:
	explicit Building(int id);

	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement);

	void fetchStoreys(const std::map<std::string,shared_ptr<ProductShapeData>>& openings);

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	std::vector<std::shared_ptr<IfcBuildingStorey>>	m_storeysOriginal;
	std::vector<BuildingStorey>						m_storeys;
};

} // namespace IFCC

#endif // IFCC_BuildingH
