#include "GEGMaterial.h"


QString GEGMaterial::layerString(double thickness) const {
	QString res = m_name;
	res += ";" + QString("%1").arg(m_density,0,'f', 2);
	res += ";" + QString("%1").arg(thickness,0,'f', 2);
	res += ";" + QString("%1").arg(m_density*thickness,0,'f', 2);
	res += ";" + QString("%1").arg(m_lambda,0,'f', 3);
	res += ";" + QString("%1").arg(thickness/m_lambda,0,'f', 3);
	res += ";" + QString("%1").arg(m_my,0,'f', 1);
	res += ";" + QString("%1").arg(m_my,0,'f', 1);
	res += ";" + QString("%1").arg(m_heatCapacity,0,'f', 0);
	res += ";;";
	return res;
}
