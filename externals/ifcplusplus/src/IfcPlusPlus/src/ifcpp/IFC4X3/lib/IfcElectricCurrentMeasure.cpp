/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcMeasureValue.h"
#include "ifcpp/IFC4X3/include/IfcElectricCurrentMeasure.h"

// TYPE IfcElectricCurrentMeasure = REAL;
IFC4X3::IfcElectricCurrentMeasure::IfcElectricCurrentMeasure( double value ) { m_value = value; }
void IFC4X3::IfcElectricCurrentMeasure::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCELECTRICCURRENTMEASURE("; }
	appendRealWithoutTrailingZeros( stream, m_value );
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcElectricCurrentMeasure> IFC4X3::IfcElectricCurrentMeasure::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcElectricCurrentMeasure>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcElectricCurrentMeasure>(); }
	shared_ptr<IfcElectricCurrentMeasure> type_object( new IfcElectricCurrentMeasure() );
	readReal( arg, type_object->m_value );
	return type_object;
}
