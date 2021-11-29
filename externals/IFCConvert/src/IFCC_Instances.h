#ifndef IFCC_InstancesH
#define IFCC_InstancesH

#include <tinyxml.h>

#include "IFCC_BuildingElement.h"
#include "IFCC_ComponentInstance.h"

namespace IFCC {

class Site;
class Database;

class Instances
{
public:
	Instances();

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	int collectComponentInstances(std::vector<BuildingElement>& elements, Database& database, const Site& site);

	std::map<int, ComponentInstance>		m_componentInstances;
	std::map<int, ComponentInstance>		m_subSurfaceComponentInstances;
};

} // namespace IFCC

#endif // IFCC_InstancesH
