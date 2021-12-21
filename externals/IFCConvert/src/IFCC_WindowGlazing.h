#ifndef IFCC_WindowGlazingH
#define IFCC_WindowGlazingH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

namespace IFCC {

/*! Class represents a glazing system (mainly for windows).*/
class WindowGlazing
{
public:
	/*! Standard constructor.
		The default model type is 'Simple' (0).
	*/
	WindowGlazing();

	/*! Write the component in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Comparison operator.*/
	friend bool operator==(const WindowGlazing& left, const WindowGlazing& right) {
		if(left.m_name != right.m_name)
			return false;
		if(left.m_notes != right.m_notes)
			return false;
		if(!IBK::near_equal(left.m_thermalTransmittance, right.m_thermalTransmittance))
			return false;
		return true;
	}

	int			m_id;					///< Window object id
	std::string	m_color;				///< Color for using in tables and views
	std::string	m_name;					///< Window name
	int			m_modelType;			///< Model type
	std::string	m_notes;				///< Some remarks
	std::string	m_manufacturer;			///< Producer of glazing system
	std::string	m_dataSource;			///< Source of window data
	double		m_thermalTransmittance;	///< Thermal transmittance in [W/m2K]
};


} // namespace IFCC

#endif // IFCC_Window_H
