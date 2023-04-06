/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcBearingTypeDisplacementEnum.h"

// TYPE IfcBearingTypeDisplacementEnum = ENUMERATION OF	(FIXED_MOVEMENT	,FREE_MOVEMENT	,GUIDED_LONGITUDINAL	,GUIDED_TRANSVERSAL	,NOTDEFINED);
void IFC4X3::IfcBearingTypeDisplacementEnum::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCBEARINGTYPEDISPLACEMENTENUM("; }
	switch( m_enum )
	{
		case ENUM_FIXED_MOVEMENT:	stream << ".FIXED_MOVEMENT."; break;
		case ENUM_FREE_MOVEMENT:	stream << ".FREE_MOVEMENT."; break;
		case ENUM_GUIDED_LONGITUDINAL:	stream << ".GUIDED_LONGITUDINAL."; break;
		case ENUM_GUIDED_TRANSVERSAL:	stream << ".GUIDED_TRANSVERSAL."; break;
		case ENUM_NOTDEFINED:	stream << ".NOTDEFINED."; break;
	}
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcBearingTypeDisplacementEnum> IFC4X3::IfcBearingTypeDisplacementEnum::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcBearingTypeDisplacementEnum>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcBearingTypeDisplacementEnum>(); }
	shared_ptr<IfcBearingTypeDisplacementEnum> type_object( new IfcBearingTypeDisplacementEnum() );
	if( std_iequal( arg, ".FIXED_MOVEMENT." ) )
	{
		type_object->m_enum = IfcBearingTypeDisplacementEnum::ENUM_FIXED_MOVEMENT;
	}
	else if( std_iequal( arg, ".FREE_MOVEMENT." ) )
	{
		type_object->m_enum = IfcBearingTypeDisplacementEnum::ENUM_FREE_MOVEMENT;
	}
	else if( std_iequal( arg, ".GUIDED_LONGITUDINAL." ) )
	{
		type_object->m_enum = IfcBearingTypeDisplacementEnum::ENUM_GUIDED_LONGITUDINAL;
	}
	else if( std_iequal( arg, ".GUIDED_TRANSVERSAL." ) )
	{
		type_object->m_enum = IfcBearingTypeDisplacementEnum::ENUM_GUIDED_TRANSVERSAL;
	}
	else if( std_iequal( arg, ".NOTDEFINED." ) )
	{
		type_object->m_enum = IfcBearingTypeDisplacementEnum::ENUM_NOTDEFINED;
	}
	return type_object;
}