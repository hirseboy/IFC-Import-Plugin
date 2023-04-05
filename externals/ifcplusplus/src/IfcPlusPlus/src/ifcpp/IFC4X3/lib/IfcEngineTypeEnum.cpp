/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcEngineTypeEnum.h"

// TYPE IfcEngineTypeEnum = ENUMERATION OF	(EXTERNALCOMBUSTION	,INTERNALCOMBUSTION	,USERDEFINED	,NOTDEFINED);
void IFC4X3::IfcEngineTypeEnum::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCENGINETYPEENUM("; }
	switch( m_enum )
	{
		case ENUM_EXTERNALCOMBUSTION:	stream << ".EXTERNALCOMBUSTION."; break;
		case ENUM_INTERNALCOMBUSTION:	stream << ".INTERNALCOMBUSTION."; break;
		case ENUM_USERDEFINED:	stream << ".USERDEFINED."; break;
		case ENUM_NOTDEFINED:	stream << ".NOTDEFINED."; break;
	}
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcEngineTypeEnum> IFC4X3::IfcEngineTypeEnum::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcEngineTypeEnum>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcEngineTypeEnum>(); }
	shared_ptr<IfcEngineTypeEnum> type_object( new IfcEngineTypeEnum() );
	if( std_iequal( arg, ".EXTERNALCOMBUSTION." ) )
	{
		type_object->m_enum = IfcEngineTypeEnum::ENUM_EXTERNALCOMBUSTION;
	}
	else if( std_iequal( arg, ".INTERNALCOMBUSTION." ) )
	{
		type_object->m_enum = IfcEngineTypeEnum::ENUM_INTERNALCOMBUSTION;
	}
	else if( std_iequal( arg, ".USERDEFINED." ) )
	{
		type_object->m_enum = IfcEngineTypeEnum::ENUM_USERDEFINED;
	}
	else if( std_iequal( arg, ".NOTDEFINED." ) )
	{
		type_object->m_enum = IfcEngineTypeEnum::ENUM_NOTDEFINED;
	}
	return type_object;
}
