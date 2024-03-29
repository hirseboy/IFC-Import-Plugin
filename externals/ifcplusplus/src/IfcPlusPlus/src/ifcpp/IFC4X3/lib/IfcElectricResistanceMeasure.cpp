/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcDerivedMeasureValue.h"
#include "ifcpp/IFC4X3/include/IfcElectricResistanceMeasure.h"

// TYPE IfcElectricResistanceMeasure = REAL;
IFC4X3::IfcElectricResistanceMeasure::IfcElectricResistanceMeasure( double value ) { m_value = value; }
void IFC4X3::IfcElectricResistanceMeasure::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCELECTRICRESISTANCEMEASURE("; }
	appendRealWithoutTrailingZeros( stream, m_value );
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcElectricResistanceMeasure> IFC4X3::IfcElectricResistanceMeasure::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcElectricResistanceMeasure>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcElectricResistanceMeasure>(); }
	shared_ptr<IfcElectricResistanceMeasure> type_object( new IfcElectricResistanceMeasure() );
	readReal( arg, type_object->m_value );
	return type_object;
}
