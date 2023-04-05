/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcFireSuppressionTerminalTypeEnum.h"

// TYPE IfcFireSuppressionTerminalTypeEnum = ENUMERATION OF	(BREECHINGINLET	,FIREHYDRANT	,FIREMONITOR	,HOSEREEL	,SPRINKLER	,SPRINKLERDEFLECTOR	,USERDEFINED	,NOTDEFINED);
void IFC4X3::IfcFireSuppressionTerminalTypeEnum::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCFIRESUPPRESSIONTERMINALTYPEENUM("; }
	switch( m_enum )
	{
		case ENUM_BREECHINGINLET:	stream << ".BREECHINGINLET."; break;
		case ENUM_FIREHYDRANT:	stream << ".FIREHYDRANT."; break;
		case ENUM_FIREMONITOR:	stream << ".FIREMONITOR."; break;
		case ENUM_HOSEREEL:	stream << ".HOSEREEL."; break;
		case ENUM_SPRINKLER:	stream << ".SPRINKLER."; break;
		case ENUM_SPRINKLERDEFLECTOR:	stream << ".SPRINKLERDEFLECTOR."; break;
		case ENUM_USERDEFINED:	stream << ".USERDEFINED."; break;
		case ENUM_NOTDEFINED:	stream << ".NOTDEFINED."; break;
	}
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcFireSuppressionTerminalTypeEnum> IFC4X3::IfcFireSuppressionTerminalTypeEnum::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcFireSuppressionTerminalTypeEnum>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcFireSuppressionTerminalTypeEnum>(); }
	shared_ptr<IfcFireSuppressionTerminalTypeEnum> type_object( new IfcFireSuppressionTerminalTypeEnum() );
	if( std_iequal( arg, ".BREECHINGINLET." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_BREECHINGINLET;
	}
	else if( std_iequal( arg, ".FIREHYDRANT." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_FIREHYDRANT;
	}
	else if( std_iequal( arg, ".FIREMONITOR." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_FIREMONITOR;
	}
	else if( std_iequal( arg, ".HOSEREEL." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_HOSEREEL;
	}
	else if( std_iequal( arg, ".SPRINKLER." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_SPRINKLER;
	}
	else if( std_iequal( arg, ".SPRINKLERDEFLECTOR." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_SPRINKLERDEFLECTOR;
	}
	else if( std_iequal( arg, ".USERDEFINED." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_USERDEFINED;
	}
	else if( std_iequal( arg, ".NOTDEFINED." ) )
	{
		type_object->m_enum = IfcFireSuppressionTerminalTypeEnum::ENUM_NOTDEFINED;
	}
	return type_object;
}
