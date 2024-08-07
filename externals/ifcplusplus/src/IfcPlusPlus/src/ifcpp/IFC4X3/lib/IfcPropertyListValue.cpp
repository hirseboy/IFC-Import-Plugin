/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcComplexProperty.h"
#include "ifcpp/IFC4X3/include/IfcExternalReferenceRelationship.h"
#include "ifcpp/IFC4X3/include/IfcIdentifier.h"
#include "ifcpp/IFC4X3/include/IfcPropertyDependencyRelationship.h"
#include "ifcpp/IFC4X3/include/IfcPropertyListValue.h"
#include "ifcpp/IFC4X3/include/IfcPropertySet.h"
#include "ifcpp/IFC4X3/include/IfcResourceApprovalRelationship.h"
#include "ifcpp/IFC4X3/include/IfcResourceConstraintRelationship.h"
#include "ifcpp/IFC4X3/include/IfcText.h"
#include "ifcpp/IFC4X3/include/IfcUnit.h"
#include "ifcpp/IFC4X3/include/IfcValue.h"

// ENTITY IfcPropertyListValue 
IFC4X3::IfcPropertyListValue::IfcPropertyListValue( int tag ) { m_tag = tag; }
void IFC4X3::IfcPropertyListValue::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCPROPERTYLISTVALUE" << "(";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Specification ) { m_Specification->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ListValues.size() > 0 )
	{
		stream << "(";
		for( size_t ii = 0; ii < m_ListValues.size(); ++ii )
		{
			if( ii > 0 )
			{
				stream << ",";
			}
			const shared_ptr<IfcValue>& type_object = m_ListValues[ii];
			if( type_object )
			{
				type_object->getStepParameter( stream, true );
			}
			else
			{
				stream << "$";
			}
		}
		stream << ")";
	}
	else { stream << "$"; }
	stream << ",";
	if( m_Unit ) { m_Unit->getStepParameter( stream, true ); } else { stream << "$" ; }
	stream << ");";
}
void IFC4X3::IfcPropertyListValue::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcPropertyListValue::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 4 ){ std::stringstream err; err << "Wrong parameter count for entity IfcPropertyListValue, expecting 4, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_Name = IfcIdentifier::createObjectFromSTEP( args[0], map, errorStream );
	m_Specification = IfcText::createObjectFromSTEP( args[1], map, errorStream );
	readSelectList( args[2], m_ListValues, map, errorStream );
	m_Unit = IfcUnit::createObjectFromSTEP( args[3], map, errorStream );
}
void IFC4X3::IfcPropertyListValue::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcSimpleProperty::getAttributes( vec_attributes );
	shared_ptr<AttributeObjectVector> ListValues_vec_object( new AttributeObjectVector() );
	std::copy( m_ListValues.begin(), m_ListValues.end(), std::back_inserter( ListValues_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "ListValues", ListValues_vec_object ) );
	vec_attributes.emplace_back( std::make_pair( "Unit", m_Unit ) );
}
void IFC4X3::IfcPropertyListValue::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcSimpleProperty::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcPropertyListValue::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcSimpleProperty::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcPropertyListValue::unlinkFromInverseCounterparts()
{
	IfcSimpleProperty::unlinkFromInverseCounterparts();
}
