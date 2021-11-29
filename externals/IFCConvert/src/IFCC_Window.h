#ifndef IFCC_WindowH
#define IFCC_WindowH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

namespace IFCC {

class Window
{
public:
	Window();

	int									m_id;
	std::string							m_color;
	std::string							m_name;
	int									m_glazingSystemId;
	int									m_methodFrame;
	int									m_methodDivider;
	std::string							m_notes;
	std::string							m_dataSource;

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	friend bool operator==(const Window& left, const Window& right) {
		if(left.m_glazingSystemId != right.m_glazingSystemId)
			return false;
		if(left.m_methodFrame != right.m_methodFrame)
			return false;
		if(left.m_methodDivider != right.m_methodDivider)
			return false;
		return true;
	}
};


} // namespace IFCC

#endif // IFCC_Window_H
