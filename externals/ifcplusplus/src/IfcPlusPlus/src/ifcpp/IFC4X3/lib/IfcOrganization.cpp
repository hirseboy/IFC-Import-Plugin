/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcActorRole.h"
#include "ifcpp/IFC4X3/include/IfcAddress.h"
#include "ifcpp/IFC4X3/include/IfcIdentifier.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcOrganization.h"
#include "ifcpp/IFC4X3/include/IfcOrganizationRelationship.h"
#include "ifcpp/IFC4X3/include/IfcPersonAndOrganization.h"
#include "ifcpp/IFC4X3/include/IfcText.h"

// ENTITY IfcOrganization 
IFC4X3::IfcOrganization::IfcOrganization( int tag ) { m_tag = tag; }
void IFC4X3::IfcOrganization::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCORGANIZATION" << "(";
	if( m_Identification ) { m_Identification->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Description ) { m_Description->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	writeEntityList( stream, m_Roles );
	stream << ",";
	writeEntityList( stream, m_Addresses );
	stream << ");";
}
void IFC4X3::IfcOrganization::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcOrganization::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 5 ){ std::stringstream err; err << "Wrong parameter count for entity IfcOrganization, expecting 5, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_Identification = IfcIdentifier::createObjectFromSTEP( args[0], map, errorStream );
	m_Name = IfcLabel::createObjectFromSTEP( args[1], map, errorStream );
	m_Description = IfcText::createObjectFromSTEP( args[2], map, errorStream );
	readEntityReferenceList( args[3], m_Roles, map, errorStream );
	readEntityReferenceList( args[4], m_Addresses, map, errorStream );
}
void IFC4X3::IfcOrganization::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	vec_attributes.emplace_back( std::make_pair( "Identification", m_Identification ) );
	vec_attributes.emplace_back( std::make_pair( "Name", m_Name ) );
	vec_attributes.emplace_back( std::make_pair( "Description", m_Description ) );
	shared_ptr<AttributeObjectVector> Roles_vec_object( new AttributeObjectVector() );
	std::copy( m_Roles.begin(), m_Roles.end(), std::back_inserter( Roles_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "Roles", Roles_vec_object ) );
	shared_ptr<AttributeObjectVector> Addresses_vec_object( new AttributeObjectVector() );
	std::copy( m_Addresses.begin(), m_Addresses.end(), std::back_inserter( Addresses_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "Addresses", Addresses_vec_object ) );
}
void IFC4X3::IfcOrganization::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	shared_ptr<AttributeObjectVector> IsRelatedBy_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_IsRelatedBy_inverse.size(); ++i )
	{
		if( !m_IsRelatedBy_inverse[i].expired() )
		{
			IsRelatedBy_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcOrganizationRelationship>( m_IsRelatedBy_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "IsRelatedBy_inverse", IsRelatedBy_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> Relates_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_Relates_inverse.size(); ++i )
	{
		if( !m_Relates_inverse[i].expired() )
		{
			Relates_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcOrganizationRelationship>( m_Relates_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "Relates_inverse", Relates_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> Engages_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_Engages_inverse.size(); ++i )
	{
		if( !m_Engages_inverse[i].expired() )
		{
			Engages_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcPersonAndOrganization>( m_Engages_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "Engages_inverse", Engages_inverse_vec_obj ) );
}
void IFC4X3::IfcOrganization::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	shared_ptr<IfcOrganization> ptr_self = dynamic_pointer_cast<IfcOrganization>( ptr_self_entity );
	if( !ptr_self ) { throw BuildingException( "IfcOrganization::setInverseCounterparts: type mismatch" ); }
	for( size_t i=0; i<m_Addresses.size(); ++i )
	{
		if( m_Addresses[i] )
		{
			m_Addresses[i]->m_OfOrganization_inverse.emplace_back( ptr_self );
		}
	}
}
void IFC4X3::IfcOrganization::unlinkFromInverseCounterparts()
{
	for( size_t i=0; i<m_Addresses.size(); ++i )
	{
		if( m_Addresses[i] )
		{
			std::vector<weak_ptr<IfcOrganization> >& OfOrganization_inverse = m_Addresses[i]->m_OfOrganization_inverse;
			for( auto it_OfOrganization_inverse = OfOrganization_inverse.begin(); it_OfOrganization_inverse != OfOrganization_inverse.end(); )
			{
				weak_ptr<IfcOrganization> self_candidate_weak = *it_OfOrganization_inverse;
				if( self_candidate_weak.expired() )
				{
					++it_OfOrganization_inverse;
					continue;
				}
				shared_ptr<IfcOrganization> self_candidate( *it_OfOrganization_inverse );
				if( self_candidate.get() == this )
				{
					it_OfOrganization_inverse= OfOrganization_inverse.erase( it_OfOrganization_inverse );
				}
				else
				{
					++it_OfOrganization_inverse;
				}
			}
		}
	}
}
