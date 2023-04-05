/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#pragma once
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include "ifcpp/model/GlobalDefines.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingObject.h"

namespace IFC4X3
{
	// TYPE IfcProjectionElementTypeEnum = ENUMERATION OF	(BLISTER	,DEVIATOR	,USERDEFINED	,NOTDEFINED);
	class IFCQUERY_EXPORT IfcProjectionElementTypeEnum : virtual public BuildingObject
	{
	public:
		enum IfcProjectionElementTypeEnumEnum
		{
			ENUM_BLISTER,
			ENUM_DEVIATOR,
			ENUM_USERDEFINED,
			ENUM_NOTDEFINED
		};

		IfcProjectionElementTypeEnum() = default;
		IfcProjectionElementTypeEnum( IfcProjectionElementTypeEnumEnum e ) { m_enum = e; }
		virtual uint32_t classID() const { return 1897649832; }
		virtual void getStepParameter( std::stringstream& stream, bool is_select_type = false ) const;
		static shared_ptr<IfcProjectionElementTypeEnum> createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream );
		IfcProjectionElementTypeEnumEnum m_enum;
	};
}
