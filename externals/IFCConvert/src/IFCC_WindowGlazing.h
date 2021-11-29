#ifndef IFCC_WindowGlazingH
#define IFCC_WindowGlazingH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

namespace IFCC {

class WindowGlazing
{
public:
	WindowGlazing();

	int									m_id;
	std::string							m_color;
	std::string							m_name;
	int									m_modelType;
	std::string							m_notes;
	std::string							m_manufacturer;
	std::string							m_dataSource;
	double								m_thermalTransmittance;

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	friend bool operator==(const WindowGlazing& left, const WindowGlazing& right) {
		if(left.m_notes != right.m_notes)
			return false;
		if(!IBK::near_equal(left.m_thermalTransmittance, right.m_thermalTransmittance))
			return false;
		return true;
	}
};


} // namespace IFCC

#endif // IFCC_Window_H
