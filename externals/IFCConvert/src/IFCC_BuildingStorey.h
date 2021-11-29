#ifndef IFCC_BuildingStoreyH
#define IFCC_BuildingStoreyH

#include <ifcpp/IFC4/include/IfcSpatialStructureElement.h>
#include <ifcpp/IFC4/include/IfcSpace.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>

#include <tinyxml.h>

#include "IFCC_EntityBase.h"
#include "IFCC_Space.h"

namespace IFCC {

class BuildingStorey : public EntityBase
{
public:
	explicit BuildingStorey(int id);

	bool set(std::shared_ptr<IfcSpatialStructureElement> ifcElement);

	void fetchSpaces(const std::map<std::string,shared_ptr<ProductShapeData>>& shapes,
					 shared_ptr<UnitConverter>& unit_converter);

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	std::vector<Space>						m_spaces;

private:
	std::vector<std::shared_ptr<IfcSpace>>	m_spacesOriginal;
};

} // namespace IFCC

#endif // IFCC_BuildingStoreyH
