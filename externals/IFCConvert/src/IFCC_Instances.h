#ifndef IFCC_InstancesH
#define IFCC_InstancesH

#include <tinyxml.h>

#include "IFCC_BuildingElement.h"
#include "IFCC_ComponentInstance.h"
#include "IFCC_BuildingElementsCollector.h"

//namespace VICUS {
//	class Project;
//}

namespace IFCC {

class Site;
class Database;

/*! Class contains and collects component instances.*/
class Instances
{
public:
	/*! Standard constructor.*/
	Instances();

	/*! Clear all the content.*/
	void clear();

	/*! Write the instances in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Collect component and subsurface component instances from building elements and other databases.
		\param elements Collection of all building element vectors
		\param database Collection of all databases
		\param site Building site which contains all geometric data (including space boundaries)
		\param errors Global convert error list
	*/
	void collectComponentInstances(BuildingElementsCollector& elements, Database& database,
								  const Site& site, std::vector<ConvertError>& errors);

//	/*! Add all component instances and sub surface component instances to the given vicus project.
//		\param idMap Map for mapping current ids to new VICUS ids.
//	*/
//	void addToVicusProject(VICUS::Project* project, std::map<int,int>& idMap);

private:

	/*! Collect component and subsurface component instances from building elements and other databases.
		\param elements Building element vector
		\param database Collection of all databases
		\param site Building site which contains all geometric data (including space boundaries)
		\param errors Global convert error list
	*/
	void collectNormalComponentInstances(BuildingElementsCollector& elements, Database& database,
								  const Site& site, std::vector<ConvertError>& errors);


	/*! Collect subsurface component instances from opening elements and component database.
		\param elements Building element vector
		\param database Collection of all databases
		\param site Building site which contains all geometric data (including space boundaries)
		\param errors Global convert error list
	*/
	void collectSubSurfaceComponentInstances(BuildingElementsCollector& elements, Database& database,
											 const Site& site, std::vector<ConvertError>& errors);
	std::map<int, ComponentInstance>		m_componentInstances;			///< Map of id and component instance
	std::map<int, ComponentInstance>		m_subSurfaceComponentInstances;	///< map of id and subsurface component instance
};

} // namespace IFCC

#endif // IFCC_InstancesH
