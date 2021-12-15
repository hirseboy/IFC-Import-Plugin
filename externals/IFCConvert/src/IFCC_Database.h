#ifndef IFCC_DatabaseH
#define IFCC_DatabaseH

#include <tinyxml.h>

#include "IFCC_Material.h"
#include "IFCC_Construction.h"
#include "IFCC_Window.h"
#include "IFCC_WindowGlazing.h"
#include "IFCC_BuildingElement.h"
#include "IFCC_Component.h"
#include "IFCC_SubSurfaceComponent.h"

namespace IFCC {

/*! Class contains all objects for databases and functions for handling.*/
class Database
{
public:
	/*! Standard constructor.*/
	Database();

	/*! Write all databases in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Create all databases from given building element vector.
		Calls collectMaterialsAndConstructions, collectWindowsAndGlazings and collectComponents.
	*/
	void collectData(std::vector<BuildingElement>& elements);


	std::map<int, Material>				m_materials;			///< Material database as map of id and material object
	std::map<int, Construction>			m_constructions;		///< Construction database as map of id and construction object
	std::map<int, Window>				m_windows;				///< Window database as map of id and window object
	std::map<int, WindowGlazing>		m_windowGlazings;		///< Glazing system database as map of id and glazing object
	std::map<int, Component>			m_components;			///< Component database as map of id and component object
	std::map<int, SubSurfaceComponent>	m_subSurfaceComponents;	///< Subsurface component database as map of id and subsurface component object

private:
	/*! Create material and construction database from given building element vector.*/
	void collectMaterialsAndConstructions(std::vector<BuildingElement>& elements);

	/*! Create window and glazing database from given building element vector.*/
	void collectWindowsAndGlazings(std::vector<BuildingElement>& elements);

	/*! Create components and subsurface components database from given building element vector.*/
	void collectComponents(std::vector<BuildingElement>& elements);

};

} // namespace IFCC

#endif // IFCC_DatabaseH
