/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#include <sstream>
#include <limits>
#include <map>
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/IFC4X3/include/IfcDerivedMeasureValue.h"
#include "ifcpp/IFC4X3/include/IfcLuminousFluxMeasure.h"

// TYPE IfcLuminousFluxMeasure = REAL;
IFC4X3::IfcLuminousFluxMeasure::IfcLuminousFluxMeasure( double value ) { m_value = value; }
void IFC4X3::IfcLuminousFluxMeasure::getStepParameter( std::stringstream& stream, bool is_select_type ) const
{
	if( is_select_type ) { stream << "IFCLUMINOUSFLUXMEASURE("; }
	appendRealWithoutTrailingZeros( stream, m_value );
	if( is_select_type ) { stream << ")"; }
}
shared_ptr<IFC4X3::IfcLuminousFluxMeasure> IFC4X3::IfcLuminousFluxMeasure::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.compare( "$" ) == 0 ) { return shared_ptr<IfcLuminousFluxMeasure>(); }
	if( arg.compare( "*" ) == 0 ) { return shared_ptr<IfcLuminousFluxMeasure>(); }
	shared_ptr<IfcLuminousFluxMeasure> type_object( new IfcLuminousFluxMeasure() );
	readReal( arg, type_object->m_value );
	return type_object;
}
