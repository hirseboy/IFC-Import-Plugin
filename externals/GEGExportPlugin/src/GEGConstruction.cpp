#include "GEGConstruction.h"

#include <VICUS_Project.h>
#include <VICUS_Construction.h>
#include <VICUS_utilities.h>
#include <VICUS_Window.h>

#include "GEG_Utilities.h"

bool GEGConstruction::set(const VICUS::Construction &constr, const VICUS::Project& project) {
	m_isWindow = false;
	m_name = fromMultiLanguageString(constr.m_displayName,"de");
	m_constructionId = constr.m_id;
	for(const auto& layer : constr.m_materialLayers) {
		double thickness = layer.m_para[VICUS::MaterialLayer::P_Thickness].value;
		const VICUS::Material* mat = VICUS::element(project.m_embeddedDB.m_materials, layer.m_idMaterial);
		if(mat != nullptr || !mat->isValid()) {
			GEGMaterial material;
			material.m_name = fromMultiLanguageString(mat->m_displayName,"de");
			material.m_producer = QString::fromStdString(mat->m_manufacturer.string());
			material.m_density = mat->m_para[VICUS::Material::P_Density].value;
			material.m_lambda = mat->m_para[VICUS::Material::P_Conductivity].value;
			material.m_heatCapacity = mat->m_para[VICUS::Material::P_HeatCapacity].value;
			material.m_my = mat->m_para[VICUS::Material::P_Mu].value;
			m_layers.push_back({thickness,material});
		}
		else {
			m_errors << QString("Non valid material in construction %1").arg(constr.m_id);
			return false;
		}
	}
	return true;
}

bool GEGConstruction::set(const VICUS::Window &window, const VICUS::Project &project) {
	m_isWindow = true;
	m_name = fromMultiLanguageString(window.m_displayName,"de");
	m_constructionId = window.m_id;
	const VICUS::WindowGlazingSystem* glazing = VICUS::element(project.m_embeddedDB.m_windowGlazingSystems, window.m_idGlazingSystem);
	if(glazing != nullptr) {
		m_glazingUValue = glazing->uValue();

		// add virtual layers
		GEGMaterial material1;
		material1.m_name = "Glas";
		material1.m_density = 0;
		material1.m_lambda = 0;
		material1.m_heatCapacity = 830;
		material1.m_my = -1500;
		m_layers.push_back({0.004,material1});

		GEGMaterial material2;
		material2.m_name = "Scheibenzwischenraum";
		material2.m_density = 0;
		material2.m_lambda = 0;
		material2.m_heatCapacity = 1080;
		material2.m_my = 1;
		m_layers.push_back({0.012,material2});

		m_layers.push_back({0.004,material1});
		return true;
	}
	else {
		m_errors << QString("Non valid glazing in window %1").arg(window.m_id);
		return false;
	}
}

double GEGConstruction::calculateUValue(double ri, double re) {
	if(!m_isWindow) {
		double RTotal = 0;
		for(const auto& layer : m_layers) {
			if(layer.second.m_lambda > 0)
				RTotal += layer.first / layer.second.m_lambda;
			else {
				m_errors << QString("At least one material in construction %1 is not valid").arg(m_constructionId);
				return 0;
			}
		}
		RTotal += ri + re;
		return 1.0 / RTotal;
	}
	else {
		if(m_glazingUValue > 0)
			return 1.0/(1.0/m_glazingUValue + ri + re);

		m_errors << QString("Glazing system in window %1 is not valid").arg(m_constructionId);
		return 0;
	}
}

QString GEGConstruction::string() const {
	QString res;
	res += "Bauteil;" + m_name + ";;\n";
	for(const auto& layer : m_layers) {
		const GEGMaterial& mat = layer.second;
		res += mat.layerString(layer.first) + "\n";
	}
	return res;
}
