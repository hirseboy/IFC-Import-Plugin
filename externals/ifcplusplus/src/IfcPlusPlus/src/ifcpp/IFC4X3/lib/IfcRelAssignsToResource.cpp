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
#include "ifcpp/IFC4X3/include/IfcObjectDefinition.h"
#include "ifcpp/IFC4X3/include/IfcObjectTypeEnum.h"
#include "ifcpp/IFC4X3/include/IfcOwnerHistory.h"
#include "ifcpp/IFC4X3/include/IfcRelAssignsToResource.h"
#include "ifcpp/IFC4X3/include/IfcResource.h"
#include "ifcpp/IFC4X3/include/IfcResourceSelect.h"
#include "ifcpp/IFC4X3/include/IfcText.h"
#include "ifcpp/IFC4X3/include/IfcTypeResource.h"

// ENTITY IfcRelAssignsToResource 
IFC4X3::IfcRelAssignsToResource::IfcRelAssignsToResource( int tag ) { m_tag = tag; }
void IFC4X3::IfcRelAssignsToResource::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCRELASSIGNSTORESOURCE" << "(";
	if( m_GlobalId ) { m_GlobalId->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_OwnerHistory ) { stream << "#" << m_OwnerHistory->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Description ) { m_Description->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	writeEntityList( stream, m_RelatedObjects );
	stream << ",";
	if( m_RelatedObjectsType ) { m_RelatedObjectsType->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_RelatingResource ) { m_RelatingResource->getStepParameter( stream, true ); } else { stream << "$" ; }
	stream << ");";
}
void IFC4X3::IfcRelAssignsToResource::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcRelAssignsToResource::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 7 ){ std::stringstream err; err << "Wrong parameter count for entity IfcRelAssignsToResource, expecting 7, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_GlobalId = IfcGloballyUniqueId::createObjectFromSTEP( args[0], map, errorStream );
	readEntityReference( args[1], m_OwnerHistory, map, errorStream );
	m_Name = IfcLabel::createObjectFromSTEP( args[2], map, errorStream );
	m_Description = IfcText::createObjectFromSTEP( args[3], map, errorStream );
	readEntityReferenceList( args[4], m_RelatedObjects, map, errorStream );
	m_RelatedObjectsType = IfcObjectTypeEnum::createObjectFromSTEP( args[5], map, errorStream );
	m_RelatingResource = IfcResourceSelect::createObjectFromSTEP( args[6], map, errorStream );
}
void IFC4X3::IfcRelAssignsToResource::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcRelAssigns::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "RelatingResource", m_RelatingResource ) );
}
void IFC4X3::IfcRelAssignsToResource::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcRelAssigns::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcRelAssignsToResource::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcRelAssigns::setInverseCounterparts( ptr_self_entity );
	shared_ptr<IfcRelAssignsToResource> ptr_self = dynamic_pointer_cast<IfcRelAssignsToResource>( ptr_self_entity );
	if( !ptr_self ) { throw BuildingException( "IfcRelAssignsToResource::setInverseCounterparts: type mismatch" ); }
	shared_ptr<IfcResource>  RelatingResource_IfcResource = dynamic_pointer_cast<IfcResource>( m_RelatingResource );
	if( RelatingResource_IfcResource )
	{
		RelatingResource_IfcResource->m_ResourceOf_inverse.emplace_back( ptr_self );
	}
	shared_ptr<IfcTypeResource>  RelatingResource_IfcTypeResource = dynamic_pointer_cast<IfcTypeResource>( m_RelatingResource );
	if( RelatingResource_IfcTypeResource )
	{
		RelatingResource_IfcTypeResource->m_ResourceOf_inverse.emplace_back( ptr_self );
	}
}
void IFC4X3::IfcRelAssignsToResource::unlinkFromInverseCounterparts()
{
	IfcRelAssigns::unlinkFromInverseCounterparts();
	shared_ptr<IfcResource>  RelatingResource_IfcResource = dynamic_pointer_cast<IfcResource>( m_RelatingResource );
	if( RelatingResource_IfcResource )
	{
		std::vector<weak_ptr<IfcRelAssignsToResource> >& ResourceOf_inverse = RelatingResource_IfcResource->m_ResourceOf_inverse;
		for( auto it_ResourceOf_inverse = ResourceOf_inverse.begin(); it_ResourceOf_inverse != ResourceOf_inverse.end(); )
		{
			weak_ptr<IfcRelAssignsToResource> self_candidate_weak = *it_ResourceOf_inverse;
			if( self_candidate_weak.expired() )
			{
				++it_ResourceOf_inverse;
				continue;
			}
			shared_ptr<IfcRelAssignsToResource> self_candidate( *it_ResourceOf_inverse );
			if( self_candidate.get() == this )
			{
				it_ResourceOf_inverse= ResourceOf_inverse.erase( it_ResourceOf_inverse );
			}
			else
			{
				++it_ResourceOf_inverse;
			}
		}
	}
	shared_ptr<IfcTypeResource>  RelatingResource_IfcTypeResource = dynamic_pointer_cast<IfcTypeResource>( m_RelatingResource );
	if( RelatingResource_IfcTypeResource )
	{
		std::vector<weak_ptr<IfcRelAssignsToResource> >& ResourceOf_inverse = RelatingResource_IfcTypeResource->m_ResourceOf_inverse;
		for( auto it_ResourceOf_inverse = ResourceOf_inverse.begin(); it_ResourceOf_inverse != ResourceOf_inverse.end(); )
		{
			weak_ptr<IfcRelAssignsToResource> self_candidate_weak = *it_ResourceOf_inverse;
			if( self_candidate_weak.expired() )
			{
				++it_ResourceOf_inverse;
				continue;
			}
			shared_ptr<IfcRelAssignsToResource> self_candidate( *it_ResourceOf_inverse );
			if( self_candidate.get() == this )
			{
				it_ResourceOf_inverse= ResourceOf_inverse.erase( it_ResourceOf_inverse );
			}
			else
			{
				++it_ResourceOf_inverse;
			}
		}
	}
}
