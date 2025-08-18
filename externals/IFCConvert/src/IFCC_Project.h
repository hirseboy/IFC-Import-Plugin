#ifndef IFCC_ProjectH
#define IFCC_ProjectH


#include <ifcpp/IFC4X3/include/IfcProject.h>
#include <ifcpp/IFC4X3/include/IfcBuilding.h>

#include <tinyxml.h>

#include "IFCC_GeometryInputData.h"
#include "IFCC_Types.h"
#include "IFCC_Surface.h"
#include "IFCC_Building.h"
#include "IFCC_EntityBase.h"

namespace IFCC {

/*! Class represents a ifc project. It can contain also buildings with their content (storeys, spaces etc.).
*/
class Project : public EntityBase
{
public:
	/*! Standard constructor.
		\param id Unique id for using in project.
	*/
	explicit Project(int id);

	/*! Initialise site from a IFC site object.
		It set a name and fills the original building vector.
		\param ifcElement IFC object for a site. It uses the base class of IfcBuildingStorey.
	*/
	bool set(std::shared_ptr<IFC4X3::IfcProject> ifcProject);

	const std::vector<std::shared_ptr<IFC4X3::IfcBuilding>>& buildingsOriginal() const;

private:

	/*! Fill the buildings vector from original buildings vector and given building shapes.
		\param buildings Shapes for all buildings in project.
	*/
	void fetchBuildings(const std::map<std::string,shared_ptr<ProductShapeData>>& buildings);

	std::vector<std::shared_ptr<Building>>	m_buildings;	///< vector of pointers to building objects

	std::vector<std::shared_ptr<IFC4X3::IfcBuilding>>	m_buildingsOriginal;	///< Vector of original IFC building objects
};

} // namespace IFCC

#endif // IFCC_ProjectH
