#ifndef GEGMaterialH
#define GEGMaterialH

#include <QString>

class GEGMaterial
{
public:
	GEGMaterial() = default;

	QString layerString(double thickness) const;

	QString	m_name;
	QString	m_producer;
	double	m_density		= 0;
	double	m_lambda		= 0;
	double	m_heatCapacity	= 0;
	double	m_my			= 0;
};

#endif // GEGMaterialH
