#ifndef IFCC_MaterialH
#define IFCC_MaterialH

#include <tinyxml.h>

namespace IFCC {

class Material
{
public:
	Material();

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	std::string m_name;
	int			m_id;
	std::string m_notes;
	std::string m_manufacturer;
	std::string m_dataSource;
};

} // namespace IFCC

#endif // IFCC_MATERIAL_H
