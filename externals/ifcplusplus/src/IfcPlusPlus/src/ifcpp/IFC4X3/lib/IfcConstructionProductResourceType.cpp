/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcAppliedValue.h"
#include "ifcpp/IFC4X3/include/IfcConstructionProductResourceType.h"
#include "ifcpp/IFC4X3/include/IfcConstructionProductResourceTypeEnum.h"
#include "ifcpp/IFC4X3/include/IfcGloballyUniqueId.h"
#include "ifcpp/IFC4X3/include/IfcIdentifier.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcOwnerHistory.h"
#include "ifcpp/IFC4X3/include/IfcPhysicalQuantity.h"
#include "ifcpp/IFC4X3/include/IfcPropertySetDefinition.h"
#include "ifcpp/IFC4X3/include/IfcRelAggregates.h"
#include "ifcpp/IFC4X3/include/IfcRelAssigns.h"
#include "ifcpp/IFC4X3/include/IfcRelAssignsToResource.h"
#include "ifcpp/IFC4X3/include/IfcRelAssociates.h"
#include "ifcpp/IFC4X3/include/IfcRelDeclares.h"
#include "ifcpp/IFC4X3/include/IfcRelDefinesByType.h"
#include "ifcpp/IFC4X3/include/IfcRelNests.h"
#include "ifcpp/IFC4X3/include/IfcText.h"

// ENTITY IfcConstructionProductResourceType 
IFC4X3::IfcConstructionProductResourceType::IfcConstructionProductResourceType( int tag ) { m_tag = tag; }
void IFC4X3::IfcConstructionProductResourceType::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCCONSTRUCTIONPRODUCTRESOURCETYPE" << "(";
	if( m_GlobalId ) { m_GlobalId->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_OwnerHistory ) { stream << "#" << m_OwnerHistory->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Description ) { m_Description->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ApplicableOccurrence ) { m_ApplicableOccurrence->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	writeEntityList( stream, m_HasPropertySets );
	stream << ",";
	if( m_Identification ) { m_Identification->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_LongDescription ) { m_LongDescription->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ResourceType ) { m_ResourceType->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	writeEntityList( stream, m_BaseCosts );
	stream << ",";
	if( m_BaseQuantity ) { stream << "#" << m_BaseQuantity->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_PredefinedType ) { m_PredefinedType->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcConstructionProductResourceType::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcConstructionProductResourceType::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 12 ){ std::stringstream err; err << "Wrong parameter count for entity IfcConstructionProductResourceType, expecting 12, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_GlobalId = IfcGloballyUniqueId::createObjectFromSTEP( args[0], map, errorStream );
	readEntityReference( args[1], m_OwnerHistory, map, errorStream );
	m_Name = IfcLabel::createObjectFromSTEP( args[2], map, errorStream );
	m_Description = IfcText::createObjectFromSTEP( args[3], map, errorStream );
	m_ApplicableOccurrence = IfcIdentifier::createObjectFromSTEP( args[4], map, errorStream );
	readEntityReferenceList( args[5], m_HasPropertySets, map, errorStream );
	m_Identification = IfcIdentifier::createObjectFromSTEP( args[6], map, errorStream );
	m_LongDescription = IfcText::createObjectFromSTEP( args[7], map, errorStream );
	m_ResourceType = IfcLabel::createObjectFromSTEP( args[8], map, errorStream );
	readEntityReferenceList( args[9], m_BaseCosts, map, errorStream );
	readEntityReference( args[10], m_BaseQuantity, map, errorStream );
	m_PredefinedType = IfcConstructionProductResourceTypeEnum::createObjectFromSTEP( args[11], map, errorStream );
}
void IFC4X3::IfcConstructionProductResourceType::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcConstructionResourceType::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "PredefinedType", m_PredefinedType ) );
}
void IFC4X3::IfcConstructionProductResourceType::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcConstructionResourceType::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcConstructionProductResourceType::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcConstructionResourceType::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcConstructionProductResourceType::unlinkFromInverseCounterparts()
{
	IfcConstructionResourceType::unlinkFromInverseCounterparts();
}
