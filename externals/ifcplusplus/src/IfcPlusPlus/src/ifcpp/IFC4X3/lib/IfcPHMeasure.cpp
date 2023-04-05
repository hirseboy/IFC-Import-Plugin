/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcDerivedMeasureValue.h"
#include "ifcpp/IFC4X3/include/IfcPHMeasure.h"

// TYPE IfcPHMeasure = REAL;
IFC4X3::IfcPHMeasure::IfcPHMeasure( double value ) { m_value = value; }
void IFC4X3::IfcPHMeasure::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCPHMEASURE("; }
	appendRealWithoutTrailingZeros( stream, m_value );
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcPHMeasure> IFC4X3::IfcPHMeasure::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcPHMeasure>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcPHMeasure>(); }
	shared_ptr<IfcPHMeasure> type_object( new IfcPHMeasure() );
	readReal( arg, type_object->m_value );
	return type_object;
}
