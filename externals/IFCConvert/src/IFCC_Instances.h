#ifndef IFCC_InstancesH
#define IFCC_InstancesH

#include <tinyxml.h>

#include "IFCC_BuildingElement.h"
#include "IFCC_ComponentInstance.h"

namespace VICUS {
	class Project;
}

namespace IFCC {

class Site;
class Database;

/*! Class contains and collects component instances.*/
class Instances
{
public:
	/*! Standard constructor.*/
	Instances();

	/*! Write the instances in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Collect component and subsurface component instances from building elements and other databases.
		\param elements Building element vector
		\param database Collection of all databases
		\param site Building site which contains all geometric data (including space boundaries)
		\return The number of surfaces from space boundaries which cannot be connected with a component
	*/
	int collectComponentInstances(std::vector<BuildingElement>& elements, Database& database, const Site& site);

	/*! Add all component instances and sub surface component instances to the given vicus project.
		\param idMap Map for mapping current ids to new VICUS ids.
	*/
	void addToVicusProject(VICUS::Project* project, std::map<int,int>& idMap);

private:

	std::map<int, ComponentInstance>		m_componentInstances;			///< Map of id and component instance
	std::map<int, ComponentInstance>		m_subSurfaceComponentInstances;	///< map of id and subsurface component instance
};

} // namespace IFCC

#endif // IFCC_InstancesH
