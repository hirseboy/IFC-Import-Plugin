#ifndef IFCC_MaterialH
#define IFCC_MaterialH

#include <tinyxml.h>

#include <map>

//#include <VICUS_Material.h>

#include "IFCC_Property.h"

namespace IFCC {

/*! Class represents a material.*/
class Material
{
public:
	/*! Standard constructor.*/
	Material();

	/*! Return the object ID.*/
	int id() const {
		return m_id;
	}

	/*! Write the component in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Comparison operator.*/
	friend bool operator==(const Material& left, const Material& right) {
		if(left.m_name != right.m_name)
			return false;
		if(left.m_notes != right.m_notes)
			return false;
		return true;
	}

	/*! Set material properties from the given property map.
		Non valid properties from the map were not be included.*/
	void setPropertiesFromPropertyMap(const std::map<std::string,std::map<std::string,Property>>& pmap);

//	/*! Create a VICUS material object and return this.
//		The returned object contains all transferable data.
//	*/
//	VICUS::Material getVicusObject(std::map<int,int>& idMap, int idOffset) const;

	std::string m_name;				///< Material name
	int			m_id;				///< Material object id
	std::string m_notes;			///< Some remarks
	std::string m_manufacturer;		///< Material producer
	std::string m_dataSource;		///< Source of material data

	// properties
	double	m_porosity;
	double	m_density;
	double	m_upperMyValue;
	double	m_lowerMyValue;
	double	m_moistureDiffusivity;
	double	m_heatCapacity;
	double	m_thermalConductivity;
	std::string	m_porosityUnit = "-";
	std::string	m_densityUnit = "kg/m3";
	std::string	m_upperMyValueUnit = "-";
	std::string	m_lowerMyValueUnit = "-";
	std::string	m_moistureDiffusivityUnit = "m3/s";
	std::string	m_heatCapacityUnit = "J/kgK";
	std::string	m_thermalConductivityUnit = "W/mK";
};

} // namespace IFCC

#endif // IFCC_MaterialH
