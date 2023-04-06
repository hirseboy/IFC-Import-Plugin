/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcSignalTypeEnum.h"

// TYPE IfcSignalTypeEnum = ENUMERATION OF	(AUDIO	,MIXED	,VISUAL	,USERDEFINED	,NOTDEFINED);
void IFC4X3::IfcSignalTypeEnum::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCSIGNALTYPEENUM("; }
	switch( m_enum )
	{
		case ENUM_AUDIO:	stream << ".AUDIO."; break;
		case ENUM_MIXED:	stream << ".MIXED."; break;
		case ENUM_VISUAL:	stream << ".VISUAL."; break;
		case ENUM_USERDEFINED:	stream << ".USERDEFINED."; break;
		case ENUM_NOTDEFINED:	stream << ".NOTDEFINED."; break;
	}
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcSignalTypeEnum> IFC4X3::IfcSignalTypeEnum::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcSignalTypeEnum>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcSignalTypeEnum>(); }
	shared_ptr<IfcSignalTypeEnum> type_object( new IfcSignalTypeEnum() );
	if( std_iequal( arg, ".AUDIO." ) )
	{
		type_object->m_enum = IfcSignalTypeEnum::ENUM_AUDIO;
	}
	else if( std_iequal( arg, ".MIXED." ) )
	{
		type_object->m_enum = IfcSignalTypeEnum::ENUM_MIXED;
	}
	else if( std_iequal( arg, ".VISUAL." ) )
	{
		type_object->m_enum = IfcSignalTypeEnum::ENUM_VISUAL;
	}
	else if( std_iequal( arg, ".USERDEFINED." ) )
	{
		type_object->m_enum = IfcSignalTypeEnum::ENUM_USERDEFINED;
	}
	else if( std_iequal( arg, ".NOTDEFINED." ) )
	{
		type_object->m_enum = IfcSignalTypeEnum::ENUM_NOTDEFINED;
	}
	return type_object;
}