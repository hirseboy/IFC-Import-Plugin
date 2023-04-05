/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcSignTypeEnum.h"

// TYPE IfcSignTypeEnum = ENUMERATION OF	(MARKER	,MIRROR	,PICTORAL	,USERDEFINED	,NOTDEFINED);
void IFC4X3::IfcSignTypeEnum::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCSIGNTYPEENUM("; }
	switch( m_enum )
	{
		case ENUM_MARKER:	stream << ".MARKER."; break;
		case ENUM_MIRROR:	stream << ".MIRROR."; break;
		case ENUM_PICTORAL:	stream << ".PICTORAL."; break;
		case ENUM_USERDEFINED:	stream << ".USERDEFINED."; break;
		case ENUM_NOTDEFINED:	stream << ".NOTDEFINED."; break;
	}
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcSignTypeEnum> IFC4X3::IfcSignTypeEnum::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcSignTypeEnum>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcSignTypeEnum>(); }
	shared_ptr<IfcSignTypeEnum> type_object( new IfcSignTypeEnum() );
	if( std_iequal( arg, ".MARKER." ) )
	{
		type_object->m_enum = IfcSignTypeEnum::ENUM_MARKER;
	}
	else if( std_iequal( arg, ".MIRROR." ) )
	{
		type_object->m_enum = IfcSignTypeEnum::ENUM_MIRROR;
	}
	else if( std_iequal( arg, ".PICTORAL." ) )
	{
		type_object->m_enum = IfcSignTypeEnum::ENUM_PICTORAL;
	}
	else if( std_iequal( arg, ".USERDEFINED." ) )
	{
		type_object->m_enum = IfcSignTypeEnum::ENUM_USERDEFINED;
	}
	else if( std_iequal( arg, ".NOTDEFINED." ) )
	{
		type_object->m_enum = IfcSignTypeEnum::ENUM_NOTDEFINED;
	}
	return type_object;
}
