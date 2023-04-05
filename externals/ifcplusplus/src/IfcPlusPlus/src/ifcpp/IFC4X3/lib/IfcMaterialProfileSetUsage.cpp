/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcCardinalPointReference.h"
#include "ifcpp/IFC4X3/include/IfcMaterialProfileSet.h"
#include "ifcpp/IFC4X3/include/IfcMaterialProfileSetUsage.h"
#include "ifcpp/IFC4X3/include/IfcPositiveLengthMeasure.h"
#include "ifcpp/IFC4X3/include/IfcRelAssociatesMaterial.h"

// ENTITY IfcMaterialProfileSetUsage 
IFC4X3::IfcMaterialProfileSetUsage::IfcMaterialProfileSetUsage( int tag ) { m_tag = tag; }
void IFC4X3::IfcMaterialProfileSetUsage::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCMATERIALPROFILESETUSAGE" << "(";
	if( m_ForProfileSet ) { stream << "#" << m_ForProfileSet->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_CardinalPoint ) { m_CardinalPoint->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ReferenceExtent ) { m_ReferenceExtent->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcMaterialProfileSetUsage::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcMaterialProfileSetUsage::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 3 ){ std::stringstream err; err << "Wrong parameter count for entity IfcMaterialProfileSetUsage, expecting 3, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	readEntityReference( args[0], m_ForProfileSet, map, errorStream );
	m_CardinalPoint = IfcCardinalPointReference::createObjectFromSTEP( args[1], map, errorStream );
	m_ReferenceExtent = IfcPositiveLengthMeasure::createObjectFromSTEP( args[2], map, errorStream );
}
void IFC4X3::IfcMaterialProfileSetUsage::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcMaterialUsageDefinition::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "ForProfileSet", m_ForProfileSet ) );
	vec_attributes.emplace_back( std::make_pair( "CardinalPoint", m_CardinalPoint ) );
	vec_attributes.emplace_back( std::make_pair( "ReferenceExtent", m_ReferenceExtent ) );
}
void IFC4X3::IfcMaterialProfileSetUsage::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcMaterialUsageDefinition::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcMaterialProfileSetUsage::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcMaterialUsageDefinition::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcMaterialProfileSetUsage::unlinkFromInverseCounterparts()
{
	IfcMaterialUsageDefinition::unlinkFromInverseCounterparts();
}
