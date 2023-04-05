/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcDerivedMeasureValue.h"
#include "ifcpp/IFC4X3/include/IfcThermalAdmittanceMeasure.h"

// TYPE IfcThermalAdmittanceMeasure = REAL;
IFC4X3::IfcThermalAdmittanceMeasure::IfcThermalAdmittanceMeasure( double value ) { m_value = value; }
void IFC4X3::IfcThermalAdmittanceMeasure::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCTHERMALADMITTANCEMEASURE("; }
	appendRealWithoutTrailingZeros( stream, m_value );
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcThermalAdmittanceMeasure> IFC4X3::IfcThermalAdmittanceMeasure::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcThermalAdmittanceMeasure>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcThermalAdmittanceMeasure>(); }
	shared_ptr<IfcThermalAdmittanceMeasure> type_object( new IfcThermalAdmittanceMeasure() );
	readReal( arg, type_object->m_value );
	return type_object;
}
