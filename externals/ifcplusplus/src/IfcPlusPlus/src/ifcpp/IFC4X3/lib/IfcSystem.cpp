/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcGloballyUniqueId.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcOwnerHistory.h"
#include "ifcpp/IFC4X3/include/IfcRelAggregates.h"
#include "ifcpp/IFC4X3/include/IfcRelAssigns.h"
#include "ifcpp/IFC4X3/include/IfcRelAssignsToGroup.h"
#include "ifcpp/IFC4X3/include/IfcRelAssociates.h"
#include "ifcpp/IFC4X3/include/IfcRelDeclares.h"
#include "ifcpp/IFC4X3/include/IfcRelDefinesByObject.h"
#include "ifcpp/IFC4X3/include/IfcRelDefinesByProperties.h"
#include "ifcpp/IFC4X3/include/IfcRelDefinesByType.h"
#include "ifcpp/IFC4X3/include/IfcRelNests.h"
#include "ifcpp/IFC4X3/include/IfcRelReferencedInSpatialStructure.h"
#include "ifcpp/IFC4X3/include/IfcRelServicesBuildings.h"
#include "ifcpp/IFC4X3/include/IfcSystem.h"
#include "ifcpp/IFC4X3/include/IfcText.h"

// ENTITY IfcSystem 
IFC4X3::IfcSystem::IfcSystem( int tag ) { m_tag = tag; }
void IFC4X3::IfcSystem::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCSYSTEM" << "(";
	if( m_GlobalId ) { m_GlobalId->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_OwnerHistory ) { stream << "#" << m_OwnerHistory->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Description ) { m_Description->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ObjectType ) { m_ObjectType->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcSystem::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcSystem::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 5 ){ std::stringstream err; err << "Wrong parameter count for entity IfcSystem, expecting 5, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_GlobalId = IfcGloballyUniqueId::createObjectFromSTEP( args[0], map, errorStream );
	readEntityReference( args[1], m_OwnerHistory, map, errorStream );
	m_Name = IfcLabel::createObjectFromSTEP( args[2], map, errorStream );
	m_Description = IfcText::createObjectFromSTEP( args[3], map, errorStream );
	m_ObjectType = IfcLabel::createObjectFromSTEP( args[4], map, errorStream );
}
void IFC4X3::IfcSystem::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcGroup::getAttributes( vec_attributes );
}
void IFC4X3::IfcSystem::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcGroup::getAttributesInverse( vec_attributes_inverse );
	shared_ptr<AttributeObjectVector> ServicesBuildings_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_ServicesBuildings_inverse.size(); ++i )
	{
		if( !m_ServicesBuildings_inverse[i].expired() )
		{
			ServicesBuildings_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelServicesBuildings>( m_ServicesBuildings_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "ServicesBuildings_inverse", ServicesBuildings_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> ServicesFacilities_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_ServicesFacilities_inverse.size(); ++i )
	{
		if( !m_ServicesFacilities_inverse[i].expired() )
		{
			ServicesFacilities_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelReferencedInSpatialStructure>( m_ServicesFacilities_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "ServicesFacilities_inverse", ServicesFacilities_inverse_vec_obj ) );
}
void IFC4X3::IfcSystem::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcGroup::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcSystem::unlinkFromInverseCounterparts()
{
	IfcGroup::unlinkFromInverseCounterparts();
}
