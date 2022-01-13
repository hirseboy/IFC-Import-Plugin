#include "IFCC_Database.h"

#include <VICUS_Project.h>

#include "IFCC_Helper.h"

namespace IFCC {

Database::Database()
{

}

TiXmlElement * Database::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("EmbeddedDatabase");
	parent->LinkEndChild(e);

	if(!m_materials.empty()) {
		TiXmlElement * child = new TiXmlElement("Materials");
		e->LinkEndChild(child);

		for(const auto& mat : m_materials) {
			mat.second.writeXML(child);
		}
	}

	if(!m_constructions.empty()) {
		TiXmlElement * child = new TiXmlElement("Constructions");
		e->LinkEndChild(child);

		for(const auto& constr : m_constructions) {
			constr.second.writeXML(child);
		}
	}

	if(!m_windows.empty()) {
		TiXmlElement * child = new TiXmlElement("Windows");
		e->LinkEndChild(child);

		for(const auto& window : m_windows) {
			window.second.writeXML(child);
		}
	}

	if(!m_windowGlazings.empty()) {
		TiXmlElement * child = new TiXmlElement("WindowGlazingSystems");
		e->LinkEndChild(child);

		for(const auto& glazing : m_windowGlazings) {
			glazing.second.writeXML(child);
		}
	}

	if(!m_components.empty()) {
		TiXmlElement * child = new TiXmlElement("Components");
		e->LinkEndChild(child);

		for(const auto& comp : m_components) {
			comp.second.writeXML(child);
		}
	}

	if(!m_subSurfaceComponents.empty()) {
		TiXmlElement * child = new TiXmlElement("SubSurfaceComponents");
		e->LinkEndChild(child);

		for(const auto& comp : m_subSurfaceComponents) {
			comp.second.writeXML(child);
		}
	}

	return e;
}

void Database::collectData(std::vector<BuildingElement>& elements) {
	collectWindowsAndGlazings(elements);
	collectMaterialsAndConstructions(elements);
	collectComponents(elements);
}

/*! Function searches for the highest Id in the given vector.
	Type in vector must contain a variable m_id
*/
template<typename T>
int getHighestId(const std::vector<T>& vect) {
	int res = 1;
	for(const auto& v : vect) {
		res = std::max<int>(res,v.m_id);
	}
	return res;
}

/*! Search for the given item in the given vector. It uses the comparison function equal() which is implemented in all child classes of VICUS::AbstractDBElement.
	\return Iterator to the found element or end() of the vector.
*/
template<typename T>
typename std::vector<T>::const_iterator findItem(const std::vector<T>& vect, const T& elem) {
	return std::find_if(vect.begin(), vect.end(),
							[elem](const auto& eit) {return eit.equal(&elem) == VICUS::AbstractDBElement::Equal; });
}

/*! Function for adding IFC convert database elements to the corresponding embedded database in a VICUS::Project.
	Two possible behaviours:
	- if the element is already included in the database only the id mapping map will be adopted
	- if the element is not in database it will be added
	The id of the new element will be higher than the highest existing id in the embedded database.
	It can be used for:
	- materials
	- constructions
	- windows
	- window glazings
	- components
	- subsurface components
*/
template<typename T, typename U>
void addItems(std::vector<T>& dbVect, const std::map<int,U>& sourceVect, std::map<int,int>& idMap) {
	int maxMatId = getHighestId(dbVect);
	for(const auto& sourceIt : sourceVect) {
		T newItem = sourceIt.second.getVicusObject(idMap, maxMatId);
		auto fit = findItem(dbVect, newItem);
		if(fit == dbVect.end())
			dbVect.push_back(newItem);
		else
			idMap[sourceIt.second.id()] = fit->m_id;
	}
}

void Database::addToVicusProject(VICUS::Project* project, std::map<int,int>& idMap) {
	addItems(project->m_embeddedDB.m_materials, m_materials, idMap);
	addItems(project->m_embeddedDB.m_constructions, m_constructions, idMap);
	addItems(project->m_embeddedDB.m_windowGlazingSystems, m_windowGlazings, idMap);
	addItems(project->m_embeddedDB.m_windows, m_windows, idMap);
	addItems(project->m_embeddedDB.m_components, m_components, idMap);
	addItems(project->m_embeddedDB.m_subSurfaceComponents, m_subSurfaceComponents, idMap);
}


void Database::collectComponents(std::vector<BuildingElement>& elements) {
	for(auto& elem : elements) {
		if(elem.isSurfaceComponent()) {
			Component comp;
			if(elem.type() == OT_Wall)
				comp.m_type = Component::CT_OutsideWall;
			else if(elem.type() == OT_Roof)
				comp.m_type = Component::CT_SlopedRoof;
			else if(elem.type() == OT_Slab)
				comp.m_type = Component::CT_FloorToAir;
			else
				comp.m_type = Component::CT_Miscellaneous;
			comp.m_constructionId = elem.m_constructionId;
			comp.m_name = elem.m_name;
			comp.m_id = GUID_maker::instance().guid();
			comp.m_guid = elem.m_guid;
			m_components[comp.m_id] = comp;
		}
		else if(elem.isSubSurfaceComponent()) {
			SubSurfaceComponent comp(GUID_maker::instance().guid(), elem.m_guid, elem.m_name);
			if(elem.type() == OT_Window) {
				comp.setWindow(elem.m_openingProperties.m_id);
			}
			else if(elem.type() == OT_Door) {
				comp.setDoor(elem.m_constructionId);
			}
			else {
				comp.setOther(elem.m_constructionId);
			}
			m_subSurfaceComponents[comp.id()] = comp;
		}
	}
}

void Database::collectMaterialsAndConstructions(std::vector<BuildingElement>& elements) {
	for(auto& elem : elements) {
		if(elem.m_materialLayers.empty())
			continue;

		Construction currentConst;
		for(size_t i=0; i<elem.m_materialLayers.size(); ++i) {
			std::string name = elem.m_materialLayers[i].second;
			auto fit = std::find_if(
						   m_materials.begin(),
						   m_materials.end(),
						   [name](const auto& mo) {return mo.second.m_name == name; });
			if(fit == m_materials.end()) {
				Material material;
				material.m_id  = GUID_maker::instance().guid();
				material.m_name = name;
				material.setPropertiesFromPropertyMap(elem.m_materialPropertyMap[i]);
				m_materials[material.m_id] = material;
				currentConst.m_layers.push_back(std::make_pair(material.m_id, elem.m_materialLayers[i].first));
			}
			else {
				currentConst.m_layers.push_back(std::make_pair(fit->first, elem.m_materialLayers[i].first));
			}
		}

		auto fit = std::find_if(
					   m_constructions.begin(),
					   m_constructions.end(),
					   [currentConst](const auto& mo) {return mo.second == currentConst; });
		if(fit == m_constructions.end()) {
			currentConst.m_id = GUID_maker::instance().guid();
			currentConst.m_name = "construction - " + std::to_string(currentConst.m_id);
			m_constructions[currentConst.m_id] = currentConst;
			elem.m_constructionId = currentConst.m_id;
		}
		else {
			elem.m_constructionId = fit->first;
		}
	}
}

void Database::collectWindowsAndGlazings(std::vector<BuildingElement>& elements) {
	for(auto& elem : elements) {
		if(!elem.m_openingProperties.m_isWindow)
			continue;

		Window window;
		window.m_name = elem.m_name;

		WindowGlazing glazing;
		for(const std::string& str : elem.m_openingProperties.m_windowConstructionTypes) {
			glazing.m_notes += str + ",";
		}
		if(!glazing.m_notes.empty()) {
			glazing.m_notes.pop_back();
		}

		auto fitGl = std::find_if(
					   m_windowGlazings.begin(),
					   m_windowGlazings.end(),
					   [glazing](const auto& gl) {return gl.second == glazing; });
		if(fitGl == m_windowGlazings.end()) {
			glazing.m_id = GUID_maker::instance().guid();
			glazing.m_name = "window glazing - " + std::to_string(glazing.m_id);
			m_windowGlazings[glazing.m_id] = glazing;
			window.m_glazingSystemId = glazing.m_id;
		}
		else {
			window.m_glazingSystemId = fitGl->first;
		}
		auto fitWi = std::find_if(
					   m_windows.begin(),
					   m_windows.end(),
					   [window](const auto& wi) {return wi.second == window; });
		if(fitWi == m_windows.end()) {
			window.m_id = GUID_maker::instance().guid();
			if(window.m_name.empty())
				window.m_name = "window glazing - " + std::to_string(window.m_id);
			m_windows[window.m_id] = window;
			elem.m_openingProperties.m_id = window.m_id;
		}
		else {
			elem.m_openingProperties.m_id = fitWi->first;
		}
	}
}


} // namespace IFCC
