#ifndef IFCC_MaterialH
#define IFCC_MaterialH

#include <tinyxml.h>

namespace IFCC {

/*! Class represents a material.*/
class Material
{
public:
	/*! Standard constructor.*/
	Material();

	/*! Write the component in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	std::string m_name;				///< Material name
	int			m_id;				///< Material object id
	std::string m_notes;			///< Some remarks
	std::string m_manufacturer;		///< Material producer
	std::string m_dataSource;		///< Source of material data
};

} // namespace IFCC

#endif // IFCC_MaterialH
