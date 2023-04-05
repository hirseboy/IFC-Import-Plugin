/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcGloballyUniqueId.h"

// TYPE IfcGloballyUniqueId = STRING(22) FIXED;
IFC4X3::IfcGloballyUniqueId::IfcGloballyUniqueId( std::string value ) { m_value = value; }
void IFC4X3::IfcGloballyUniqueId::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCGLOBALLYUNIQUEID("; }
	stream << "'" << encodeStepString( m_value ) << "'";
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcGloballyUniqueId> IFC4X3::IfcGloballyUniqueId::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcGloballyUniqueId>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcGloballyUniqueId>(); }
	shared_ptr<IfcGloballyUniqueId> type_object( new IfcGloballyUniqueId() );
	readString( arg, type_object->m_value );
	return type_object;
}
