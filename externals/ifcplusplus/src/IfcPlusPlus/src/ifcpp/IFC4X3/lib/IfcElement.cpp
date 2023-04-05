/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcElement.h"
#include "ifcpp/IFC4X3/include/IfcGloballyUniqueId.h"
#include "ifcpp/IFC4X3/include/IfcIdentifier.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcObjectPlacement.h"
#include "ifcpp/IFC4X3/include/IfcOwnerHistory.h"
#include "ifcpp/IFC4X3/include/IfcProductRepresentation.h"
#include "ifcpp/IFC4X3/include/IfcRelAdheresToElement.h"
#include "ifcpp/IFC4X3/include/IfcRelAggregates.h"
#include "ifcpp/IFC4X3/include/IfcRelAssigns.h"
#include "ifcpp/IFC4X3/include/IfcRelAssignsToProduct.h"
#include "ifcpp/IFC4X3/include/IfcRelAssociates.h"
#include "ifcpp/IFC4X3/include/IfcRelConnectsElements.h"
#include "ifcpp/IFC4X3/include/IfcRelConnectsWithRealizingElements.h"
#include "ifcpp/IFC4X3/include/IfcRelContainedInSpatialStructure.h"
#include "ifcpp/IFC4X3/include/IfcRelCoversBldgElements.h"
#include "ifcpp/IFC4X3/include/IfcRelDeclares.h"
#include "ifcpp/IFC4X3/include/IfcRelDefinesByObject.h"
#include "ifcpp/IFC4X3/include/IfcRelDefinesByProperties.h"
#include "ifcpp/IFC4X3/include/IfcRelDefinesByType.h"
#include "ifcpp/IFC4X3/include/IfcRelFillsElement.h"
#include "ifcpp/IFC4X3/include/IfcRelInterferesElements.h"
#include "ifcpp/IFC4X3/include/IfcRelNests.h"
#include "ifcpp/IFC4X3/include/IfcRelPositions.h"
#include "ifcpp/IFC4X3/include/IfcRelProjectsElement.h"
#include "ifcpp/IFC4X3/include/IfcRelReferencedInSpatialStructure.h"
#include "ifcpp/IFC4X3/include/IfcRelSpaceBoundary.h"
#include "ifcpp/IFC4X3/include/IfcRelVoidsElement.h"
#include "ifcpp/IFC4X3/include/IfcText.h"

// ENTITY IfcElement 
IFC4X3::IfcElement::IfcElement( int tag ) { m_tag = tag; }
void IFC4X3::IfcElement::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCELEMENT" << "(";
	if( m_GlobalId ) { m_GlobalId->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_OwnerHistory ) { stream << "#" << m_OwnerHistory->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Description ) { m_Description->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ObjectType ) { m_ObjectType->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ObjectPlacement ) { stream << "#" << m_ObjectPlacement->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Representation ) { stream << "#" << m_Representation->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Tag ) { m_Tag->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcElement::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcElement::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 8 ){ std::stringstream err; err << "Wrong parameter count for entity IfcElement, expecting 8, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_GlobalId = IfcGloballyUniqueId::createObjectFromSTEP( args[0], map, errorStream );
	readEntityReference( args[1], m_OwnerHistory, map, errorStream );
	m_Name = IfcLabel::createObjectFromSTEP( args[2], map, errorStream );
	m_Description = IfcText::createObjectFromSTEP( args[3], map, errorStream );
	m_ObjectType = IfcLabel::createObjectFromSTEP( args[4], map, errorStream );
	readEntityReference( args[5], m_ObjectPlacement, map, errorStream );
	readEntityReference( args[6], m_Representation, map, errorStream );
	m_Tag = IfcIdentifier::createObjectFromSTEP( args[7], map, errorStream );
}
void IFC4X3::IfcElement::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcProduct::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "Tag", m_Tag ) );
}
void IFC4X3::IfcElement::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcProduct::getAttributesInverse( vec_attributes_inverse );
	shared_ptr<AttributeObjectVector> FillsVoids_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_FillsVoids_inverse.size(); ++i )
	{
		if( !m_FillsVoids_inverse[i].expired() )
		{
			FillsVoids_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelFillsElement>( m_FillsVoids_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "FillsVoids_inverse", FillsVoids_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> ConnectedTo_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_ConnectedTo_inverse.size(); ++i )
	{
		if( !m_ConnectedTo_inverse[i].expired() )
		{
			ConnectedTo_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelConnectsElements>( m_ConnectedTo_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "ConnectedTo_inverse", ConnectedTo_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> IsInterferedByElements_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_IsInterferedByElements_inverse.size(); ++i )
	{
		if( !m_IsInterferedByElements_inverse[i].expired() )
		{
			IsInterferedByElements_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelInterferesElements>( m_IsInterferedByElements_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "IsInterferedByElements_inverse", IsInterferedByElements_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> InterferesElements_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_InterferesElements_inverse.size(); ++i )
	{
		if( !m_InterferesElements_inverse[i].expired() )
		{
			InterferesElements_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelInterferesElements>( m_InterferesElements_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "InterferesElements_inverse", InterferesElements_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> HasProjections_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_HasProjections_inverse.size(); ++i )
	{
		if( !m_HasProjections_inverse[i].expired() )
		{
			HasProjections_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelProjectsElement>( m_HasProjections_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "HasProjections_inverse", HasProjections_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> HasOpenings_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_HasOpenings_inverse.size(); ++i )
	{
		if( !m_HasOpenings_inverse[i].expired() )
		{
			HasOpenings_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelVoidsElement>( m_HasOpenings_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "HasOpenings_inverse", HasOpenings_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> IsConnectionRealization_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_IsConnectionRealization_inverse.size(); ++i )
	{
		if( !m_IsConnectionRealization_inverse[i].expired() )
		{
			IsConnectionRealization_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelConnectsWithRealizingElements>( m_IsConnectionRealization_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "IsConnectionRealization_inverse", IsConnectionRealization_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> ProvidesBoundaries_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_ProvidesBoundaries_inverse.size(); ++i )
	{
		if( !m_ProvidesBoundaries_inverse[i].expired() )
		{
			ProvidesBoundaries_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelSpaceBoundary>( m_ProvidesBoundaries_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "ProvidesBoundaries_inverse", ProvidesBoundaries_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> ConnectedFrom_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_ConnectedFrom_inverse.size(); ++i )
	{
		if( !m_ConnectedFrom_inverse[i].expired() )
		{
			ConnectedFrom_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelConnectsElements>( m_ConnectedFrom_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "ConnectedFrom_inverse", ConnectedFrom_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> ContainedInStructure_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_ContainedInStructure_inverse.size(); ++i )
	{
		if( !m_ContainedInStructure_inverse[i].expired() )
		{
			ContainedInStructure_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelContainedInSpatialStructure>( m_ContainedInStructure_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "ContainedInStructure_inverse", ContainedInStructure_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> HasCoverings_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_HasCoverings_inverse.size(); ++i )
	{
		if( !m_HasCoverings_inverse[i].expired() )
		{
			HasCoverings_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelCoversBldgElements>( m_HasCoverings_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "HasCoverings_inverse", HasCoverings_inverse_vec_obj ) );
	shared_ptr<AttributeObjectVector> HasSurfaceFeatures_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_HasSurfaceFeatures_inverse.size(); ++i )
	{
		if( !m_HasSurfaceFeatures_inverse[i].expired() )
		{
			HasSurfaceFeatures_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcRelAdheresToElement>( m_HasSurfaceFeatures_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "HasSurfaceFeatures_inverse", HasSurfaceFeatures_inverse_vec_obj ) );
}
void IFC4X3::IfcElement::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcProduct::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcElement::unlinkFromInverseCounterparts()
{
	IfcProduct::unlinkFromInverseCounterparts();
}
