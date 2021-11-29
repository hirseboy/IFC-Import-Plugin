#ifndef IFCC_ComponentH
#define IFCC_ComponentH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

namespace IFCC {

class Surface;

class Component
{
public:
	/*! Component types. */
	enum ComponentType {
		CT_OutsideWall,				// Keyword: OutsideWall				'Outside wall construction'
		CT_OutsideWallToGround,		// Keyword: OutsideWallToGround		'Outside wall construction in contact with ground'
		CT_InsideWall,				// Keyword: InsideWall				'Interior construction'
		CT_FloorToCellar,			// Keyword: FloorToCellar			'Floor to basement'
		CT_FloorToAir,				// Keyword: FloorToAir				'Floor in contact with air'
		CT_FloorToGround,			// Keyword: FloorToGround			'Floor in contact with ground'
		CT_Ceiling,					// Keyword: Ceiling					'Ceiling construction'
		CT_SlopedRoof,				// Keyword: SlopedRoof				'Sloped roof construction'
		CT_FlatRoof,				// Keyword: FlatRoof				'Flat roof construction'
		CT_ColdRoof,				// Keyword: ColdRoof				'Flat roof construction (to heated/insulated space)'
		CT_WarmRoof,				// Keyword: WarmRoof				'Flat roof construction (to cold/ventilated space)'
		CT_Miscellaneous,			// Keyword: Miscellaneous			'Some other component type'
		NUM_CT
	};

	Component();

	int									m_id;
	std::string							m_color;
	std::string							m_name;
	int									m_constructionId;
	std::string							m_notes;
	std::string							m_dataSource;
	std::string							m_manufacturer;
	ComponentType						m_type;
	std::string							m_guid;

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	void updateComponentType(const Surface& surf);

	friend bool operator==(const Component& left, const Component& right) {
		if(left.m_constructionId != right.m_constructionId)
			return false;
		if(left.m_type != right.m_type)
			return false;
		return true;
	}

private:
	std::string type2String(ComponentType type) const;
};


} // namespace IFCC

#endif // IFCC_Component_H
