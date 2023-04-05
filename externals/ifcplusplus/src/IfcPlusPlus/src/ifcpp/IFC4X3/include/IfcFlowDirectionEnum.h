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
	// TYPE IfcFlowDirectionEnum = ENUMERATION OF	(SINK	,SOURCE	,SOURCEANDSINK	,NOTDEFINED);
	class IFCQUERY_EXPORT IfcFlowDirectionEnum : virtual public BuildingObject
	{
	public:
		enum IfcFlowDirectionEnumEnum
		{
			ENUM_SINK,
			ENUM_SOURCE,
			ENUM_SOURCEANDSINK,
			ENUM_NOTDEFINED
		};

		IfcFlowDirectionEnum() = default;
		IfcFlowDirectionEnum( IfcFlowDirectionEnumEnum e ) { m_enum = e; }
		virtual uint32_t classID() const { return 2183683140; }
		virtual void getStepParameter( std::stringstream& stream, bool is_select_type = false ) const;
		static shared_ptr<IfcFlowDirectionEnum> createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream );
		IfcFlowDirectionEnumEnum m_enum;
	};
}
