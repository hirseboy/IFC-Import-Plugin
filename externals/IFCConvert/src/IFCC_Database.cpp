#include "IFCC_Database.h"
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
			SubSurfaceComponent comp;
			if(elem.type() == OT_Window) {
				comp.m_type = SubSurfaceComponent::CT_Window;
				comp.m_windowId = elem.m_openingProperties.m_id;
			}
			else if(elem.type() == OT_Door) {
				comp.m_type = SubSurfaceComponent::CT_Door;
				comp.m_constructionId = elem.m_constructionId;
			}
			else {
				comp.m_type = SubSurfaceComponent::CT_Miscellaneous;
				comp.m_constructionId = elem.m_constructionId;
			}
			comp.m_name = elem.m_name;
			comp.m_id = GUID_maker::instance().guid();
			comp.m_guid = elem.m_guid;
			m_subSurfaceComponents[comp.m_id] = comp;
		}
	}
}

void Database::collectMaterialsAndConstructions(std::vector<BuildingElement>& elements) {
	for(auto& elem : elements) {
		if(elem.m_materialLayers.empty())
			continue;

		Construction currentConst;
		for(const auto& mat : elem.m_materialLayers) {
			std::string name = mat.second;
			auto fit = std::find_if(
						   m_materials.begin(),
						   m_materials.end(),
						   [name](const auto& mo) {return mo.second.m_name == name; });
			if(fit == m_materials.end()) {
				Material material;
				material.m_id  = GUID_maker::instance().guid();
				material.m_name = name;
				m_materials[material.m_id] = material;
				currentConst.m_layers.push_back(std::make_pair(material.m_id, mat.first));
			}
			else {
				currentConst.m_layers.push_back(std::make_pair(fit->first, mat.first));
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
