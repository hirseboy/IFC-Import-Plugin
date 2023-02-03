#ifndef IFCC_WindowH
#define IFCC_WindowH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

//#include <VICUS_Window.h>

namespace IFCC {

/*! Class represents a window with properties and connection to glazing system.*/
class Window
{
public:
	/*! Standard constructor.
		The default methods for frame and divider are unknown (3).
	*/
	Window();

	/*! Return the object ID.*/
	int id() const {
		return m_id;
	}

	/*! Write the component in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Comparison operator.*/
	friend bool operator==(const Window& left, const Window& right) {
		if(left.m_glazingSystemId != right.m_glazingSystemId)
			return false;
		if(left.m_methodFrame != right.m_methodFrame)
			return false;
		if(left.m_methodDivider != right.m_methodDivider)
			return false;
		return true;
	}

//	/*! Create a VICUS window object and return this.
//		The returned object contains all transferable data.
//	*/
//	VICUS::Window getVicusObject(std::map<int,int>& idMap, int idOffset) const;

	int				m_id;				///< Window object id
	std::string		m_color;			///< Color for using in tables and views
	std::string		m_name;				///< Window name
	int				m_glazingSystemId;	///< Id of the used glazing system
	int				m_methodFrame;		///< Method for frame
	int				m_methodDivider;	///< method for divider
	std::string		m_notes;			///< Some remarks
	std::string		m_dataSource;		///< Source of window data
};


} // namespace IFCC

#endif // IFCC_Window_H
