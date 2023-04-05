/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcAxis2Placement3D.h"
#include "ifcpp/IFC4X3/include/IfcBoundaryCondition.h"
#include "ifcpp/IFC4X3/include/IfcGloballyUniqueId.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcLengthMeasure.h"
#include "ifcpp/IFC4X3/include/IfcOwnerHistory.h"
#include "ifcpp/IFC4X3/include/IfcRelConnectsStructuralMember.h"
#include "ifcpp/IFC4X3/include/IfcStructuralConnection.h"
#include "ifcpp/IFC4X3/include/IfcStructuralConnectionCondition.h"
#include "ifcpp/IFC4X3/include/IfcStructuralMember.h"
#include "ifcpp/IFC4X3/include/IfcText.h"

// ENTITY IfcRelConnectsStructuralMember 
IFC4X3::IfcRelConnectsStructuralMember::IfcRelConnectsStructuralMember( int tag ) { m_tag = tag; }
void IFC4X3::IfcRelConnectsStructuralMember::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCRELCONNECTSSTRUCTURALMEMBER" << "(";
	if( m_GlobalId ) { m_GlobalId->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_OwnerHistory ) { stream << "#" << m_OwnerHistory->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Description ) { m_Description->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_RelatingStructuralMember ) { stream << "#" << m_RelatingStructuralMember->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_RelatedStructuralConnection ) { stream << "#" << m_RelatedStructuralConnection->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_AppliedCondition ) { stream << "#" << m_AppliedCondition->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_AdditionalConditions ) { stream << "#" << m_AdditionalConditions->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_SupportedLength ) { m_SupportedLength->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ConditionCoordinateSystem ) { stream << "#" << m_ConditionCoordinateSystem->m_tag; } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcRelConnectsStructuralMember::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcRelConnectsStructuralMember::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 10 ){ std::stringstream err; err << "Wrong parameter count for entity IfcRelConnectsStructuralMember, expecting 10, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_GlobalId = IfcGloballyUniqueId::createObjectFromSTEP( args[0], map, errorStream );
	readEntityReference( args[1], m_OwnerHistory, map, errorStream );
	m_Name = IfcLabel::createObjectFromSTEP( args[2], map, errorStream );
	m_Description = IfcText::createObjectFromSTEP( args[3], map, errorStream );
	readEntityReference( args[4], m_RelatingStructuralMember, map, errorStream );
	readEntityReference( args[5], m_RelatedStructuralConnection, map, errorStream );
	readEntityReference( args[6], m_AppliedCondition, map, errorStream );
	readEntityReference( args[7], m_AdditionalConditions, map, errorStream );
	m_SupportedLength = IfcLengthMeasure::createObjectFromSTEP( args[8], map, errorStream );
	readEntityReference( args[9], m_ConditionCoordinateSystem, map, errorStream );
}
void IFC4X3::IfcRelConnectsStructuralMember::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcRelConnects::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "RelatingStructuralMember", m_RelatingStructuralMember ) );
	vec_attributes.emplace_back( std::make_pair( "RelatedStructuralConnection", m_RelatedStructuralConnection ) );
	vec_attributes.emplace_back( std::make_pair( "AppliedCondition", m_AppliedCondition ) );
	vec_attributes.emplace_back( std::make_pair( "AdditionalConditions", m_AdditionalConditions ) );
	vec_attributes.emplace_back( std::make_pair( "SupportedLength", m_SupportedLength ) );
	vec_attributes.emplace_back( std::make_pair( "ConditionCoordinateSystem", m_ConditionCoordinateSystem ) );
}
void IFC4X3::IfcRelConnectsStructuralMember::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcRelConnects::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcRelConnectsStructuralMember::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcRelConnects::setInverseCounterparts( ptr_self_entity );
	shared_ptr<IfcRelConnectsStructuralMember> ptr_self = dynamic_pointer_cast<IfcRelConnectsStructuralMember>( ptr_self_entity );
	if( !ptr_self ) { throw BuildingException( "IfcRelConnectsStructuralMember::setInverseCounterparts: type mismatch" ); }
	if( m_RelatedStructuralConnection )
	{
		m_RelatedStructuralConnection->m_ConnectsStructuralMembers_inverse.emplace_back( ptr_self );
	}
	if( m_RelatingStructuralMember )
	{
		m_RelatingStructuralMember->m_ConnectedBy_inverse.emplace_back( ptr_self );
	}
}
void IFC4X3::IfcRelConnectsStructuralMember::unlinkFromInverseCounterparts()
{
	IfcRelConnects::unlinkFromInverseCounterparts();
	if( m_RelatedStructuralConnection )
	{
		std::vector<weak_ptr<IfcRelConnectsStructuralMember> >& ConnectsStructuralMembers_inverse = m_RelatedStructuralConnection->m_ConnectsStructuralMembers_inverse;
		for( auto it_ConnectsStructuralMembers_inverse = ConnectsStructuralMembers_inverse.begin(); it_ConnectsStructuralMembers_inverse != ConnectsStructuralMembers_inverse.end(); )
		{
			weak_ptr<IfcRelConnectsStructuralMember> self_candidate_weak = *it_ConnectsStructuralMembers_inverse;
			if( self_candidate_weak.expired() )
			{
				++it_ConnectsStructuralMembers_inverse;
				continue;
			}
			shared_ptr<IfcRelConnectsStructuralMember> self_candidate( *it_ConnectsStructuralMembers_inverse );
			if( self_candidate.get() == this )
			{
				it_ConnectsStructuralMembers_inverse= ConnectsStructuralMembers_inverse.erase( it_ConnectsStructuralMembers_inverse );
			}
			else
			{
				++it_ConnectsStructuralMembers_inverse;
			}
		}
	}
	if( m_RelatingStructuralMember )
	{
		std::vector<weak_ptr<IfcRelConnectsStructuralMember> >& ConnectedBy_inverse = m_RelatingStructuralMember->m_ConnectedBy_inverse;
		for( auto it_ConnectedBy_inverse = ConnectedBy_inverse.begin(); it_ConnectedBy_inverse != ConnectedBy_inverse.end(); )
		{
			weak_ptr<IfcRelConnectsStructuralMember> self_candidate_weak = *it_ConnectedBy_inverse;
			if( self_candidate_weak.expired() )
			{
				++it_ConnectedBy_inverse;
				continue;
			}
			shared_ptr<IfcRelConnectsStructuralMember> self_candidate( *it_ConnectedBy_inverse );
			if( self_candidate.get() == this )
			{
				it_ConnectedBy_inverse= ConnectedBy_inverse.erase( it_ConnectedBy_inverse );
			}
			else
			{
				++it_ConnectedBy_inverse;
			}
		}
	}
}
