#ifndef GEGConstructionH
#define GEGConstructionH

#include "GEGMaterial.h"

#include <vector>

#include <QString>
#include <QStringList>

namespace VICUS {
	class Construction;
	class Project;
	class Window;
}

class GEGConstruction
{
public:
	GEGConstruction(int id = -1) :
		m_id(id)
	{}

	bool set(const VICUS::Construction& constr, const VICUS::Project& project);

	bool set(const VICUS::Window& window, const VICUS::Project& project);

	double calculateUValue(double ri, double re);

	bool valid() const {
		return m_constructionId > 0;
	}

	int											m_id;
	int											m_typeId = 3;
	int											m_constructionId = -1;
	QString										m_name;
	std::vector<std::pair<double,GEGMaterial>>	m_layers;
	QStringList									m_errors;
	bool										m_isWindow = false;
	double										m_glazingUValue = 0;
};

#endif // GEGConstructionH
