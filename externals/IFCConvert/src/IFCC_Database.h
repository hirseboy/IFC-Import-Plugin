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

class Database
{
public:
	Database();

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	void collectData(std::vector<BuildingElement>& elements);


	std::map<int, Material>				m_materials;
	std::map<int, Construction>			m_constructions;
	std::map<int, Window>				m_windows;
	std::map<int, WindowGlazing>		m_windowGlazings;
	std::map<int, Component>			m_components;
	std::map<int, SubSurfaceComponent>	m_subSurfaceComponents;

private:
	void collectMaterialsAndConstructions(std::vector<BuildingElement>& elements);
	void collectWindowsAndGlazings(std::vector<BuildingElement>& elements);
	void collectComponents(std::vector<BuildingElement>& elements);

};

} // namespace IFCC

#endif // IFCC_DatabaseH
