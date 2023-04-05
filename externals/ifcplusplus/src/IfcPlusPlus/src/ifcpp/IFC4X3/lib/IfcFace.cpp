/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcFace.h"
#include "ifcpp/IFC4X3/include/IfcFaceBound.h"
#include "ifcpp/IFC4X3/include/IfcPresentationLayerAssignment.h"
#include "ifcpp/IFC4X3/include/IfcStyledItem.h"
#include "ifcpp/IFC4X3/include/IfcTextureMap.h"

// ENTITY IfcFace 
IFC4X3::IfcFace::IfcFace( int tag ) { m_tag = tag; }
void IFC4X3::IfcFace::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCFACE" << "(";
	writeEntityList( stream, m_Bounds );
	stream << ");";
}
void IFC4X3::IfcFace::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcFace::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 1 ){ std::stringstream err; err << "Wrong parameter count for entity IfcFace, expecting 1, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	readEntityReferenceList( args[0], m_Bounds, map, errorStream );
}
void IFC4X3::IfcFace::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcTopologicalRepresentationItem::getAttributes( vec_attributes );
	shared_ptr<AttributeObjectVector> Bounds_vec_object( new AttributeObjectVector() );
	std::copy( m_Bounds.begin(), m_Bounds.end(), std::back_inserter( Bounds_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "Bounds", Bounds_vec_object ) );
}
void IFC4X3::IfcFace::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcTopologicalRepresentationItem::getAttributesInverse( vec_attributes_inverse );
	shared_ptr<AttributeObjectVector> HasTextureMaps_inverse_vec_obj( new AttributeObjectVector() );
	for( size_t i=0; i<m_HasTextureMaps_inverse.size(); ++i )
	{
		if( !m_HasTextureMaps_inverse[i].expired() )
		{
			HasTextureMaps_inverse_vec_obj->m_vec.emplace_back( shared_ptr<IfcTextureMap>( m_HasTextureMaps_inverse[i] ) );
		}
	}
	vec_attributes_inverse.emplace_back( std::make_pair( "HasTextureMaps_inverse", HasTextureMaps_inverse_vec_obj ) );
}
void IFC4X3::IfcFace::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcTopologicalRepresentationItem::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcFace::unlinkFromInverseCounterparts()
{
	IfcTopologicalRepresentationItem::unlinkFromInverseCounterparts();
}
