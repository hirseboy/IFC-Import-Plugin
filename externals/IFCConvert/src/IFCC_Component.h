#ifndef IFCC_ComponentH
#define IFCC_ComponentH

#include <vector>
#include <string>
#include <map>

//#include <VICUS_Component.h>

#include "IFCC_Types.h"

class TiXmlElement;

namespace IFCC {

class Surface;
class SpaceBoundary;

/*! Class contains all informations for a component.
	A component connects a construction with boundary conditions and further informations.
*/
class Component {
public:
	/*! Component types. */
	enum ComponentType {
		CT_OutsideWall,				///< Outside wall construction
		CT_OutsideWallToGround,		///< Outside wall construction in contact with ground
		CT_InsideWall,				///< Interior construction
		CT_FloorToCellar,			///< Floor to basement
		CT_FloorToAir,				///< Floor in contact with air
		CT_FloorToGround,			///< Floor in contact with ground
		CT_Ceiling,					///< Ceiling construction
		CT_SlopedRoof,				///</ Sloped roof construction
		CT_FlatRoof,				///< Flat roof construction
		CT_ColdRoof,				///< Flat roof construction (to heated/insulated space)
		CT_WarmRoof,				///< Flat roof construction (to cold/ventilated space)
		CT_Miscellaneous,			///< Some other component type
		NUM_CT
	};

	/*! Standard constructor.*/
	Component();

	int									m_id;				///< Id of the component
	std::string							m_color;			///< Color for table
	std::string							m_name;				///< Component name
	int									m_constructionId;	///< Id of used construction
	std::string							m_notes;			///< Comments
	std::string							m_dataSource;		///< Source of data
	std::string							m_manufacturer;		///< producer name
	ComponentType						m_type;				///< Component type
	std::string							m_guid;				///< GUID of the corresponding building element
	BasicType							m_basictype;		///< Basic type of component


	/*! Return the object ID.*/
	int id() const {
		return m_id;
	}

	/*! Write the component in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Refining of type based on basic type (CT_OutsideWall, CT_SlopedRoof and CT_FloorToAir) and surface type.*/
	void updateComponentType(const Surface& surf);

	/*! Refining of type based on basic type (CT_OutsideWall, CT_SlopedRoof and CT_FloorToAir) and space boundary type.*/
	void updateComponentType(const SpaceBoundary& sb);

	/*! Comparison operator.*/
	friend bool operator==(const Component& left, const Component& right) {
		if(left.m_constructionId != right.m_constructionId)
			return false;
		if(left.m_type != right.m_type)
			return false;
		return true;
	}

//	/*! Create a VICUS component object and return this.
//		The returned object contains all transferable data.
//	*/
//	VICUS::Component getVicusObject(std::map<int,int>& idMap, int idOffset) const;

private:
	/*! Create name of component type for writing.*/
	std::string type2String(ComponentType type) const;
};


} // namespace IFCC

#endif // IFCC_Component_H
