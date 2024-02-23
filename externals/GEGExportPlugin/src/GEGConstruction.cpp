#include "GEGConstruction.h"

#include <VICUS_Project.h>
#include <VICUS_Construction.h>
#include <VICUS_utilities.h>

void GEGConstruction::set(const VICUS::Construction &constr, const VICUS::Project& project) {
	m_name = QString::fromStdString(constr.m_displayName.string());
	m_constructionId = constr.m_id;
	for(const auto& layer : constr.m_materialLayers) {
		double thickness = layer.m_para[VICUS::MaterialLayer::P_Thickness].value;
		const VICUS::Material* mat = VICUS::element(project.m_embeddedDB.m_materials, layer.m_idMaterial);
		if(mat != nullptr) {
			GEGMaterial material;
			material.m_name = QString::fromStdString(mat->m_displayName.string());
			material.m_producer = QString::fromStdString(mat->m_manufacturer.string());
			material.m_density = mat->m_para[VICUS::Material::P_Density].value;
			material.m_lambda = mat->m_para[VICUS::Material::P_Conductivity].value;
			material.m_heatCapacity = mat->m_para[VICUS::Material::P_HeatCapacity].value;
			material.m_my = mat->m_para[VICUS::Material::P_Mu].value;
			m_layers.push_back({thickness,material});
		}
		else {
			m_errors << QString("Non valid material in construction %1").arg(constr.m_id);
		}
	}
}

double GEGConstruction::calculateUValue(double ri, double re) {
	double RTotal = 0;
	for(const auto& layer : m_layers) {
		if(layer.second.m_lambda > 0)
			RTotal += layer.first / layer.second.m_lambda;
		else {
			RTotal = 1e30;
			m_errors << QString("At least one material in construction %1 is not valid").arg(m_constructionId);
			break;
		}
	}
	RTotal += ri + re;
	return RTotal;
}
