#ifndef IFCC_SubSurfaceComponentH
#define IFCC_SubSurfaceComponentH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

namespace IFCC {

class SubSurfaceComponent
{
public:

	/*! SubSurfaceComponent types. */
	enum SubSurfaceComponentType {
		CT_Window,					// Keyword: Window					'A window'
		CT_Door,					// Keyword: Door					'A door'
		CT_Miscellaneous,			// Keyword: Miscellaneous			'Some other component type'
		NUM_CT
	};

	SubSurfaceComponent();

	int									m_id;
	std::string							m_color;
	std::string							m_name;
	SubSurfaceComponentType				m_type;
	int									m_windowId;
	int									m_constructionId;
	std::string							m_guid;

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	friend bool operator==(const SubSurfaceComponent& left, const SubSurfaceComponent& right) {
		if(left.m_type != right.m_type)
			return false;
		if(left.m_windowId != right.m_windowId)
			return false;
		if(left.m_constructionId != right.m_constructionId)
			return false;
		return true;
	}

private:
	std::string type2String(SubSurfaceComponentType type) const;
};


} // namespace IFCC

#endif // IFCC_SubSurfaceComponent_H
