/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcFillAreaStyleTiles.h"
#include "ifcpp/IFC4X3/include/IfcPositiveRatioMeasure.h"
#include "ifcpp/IFC4X3/include/IfcPresentationLayerAssignment.h"
#include "ifcpp/IFC4X3/include/IfcStyledItem.h"
#include "ifcpp/IFC4X3/include/IfcVector.h"

// ENTITY IfcFillAreaStyleTiles 
IFC4X3::IfcFillAreaStyleTiles::IfcFillAreaStyleTiles( int tag ) { m_tag = tag; }
void IFC4X3::IfcFillAreaStyleTiles::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCFILLAREASTYLETILES" << "(";
	writeEntityList( stream, m_TilingPattern );
	stream << ",";
	writeEntityList( stream, m_Tiles );
	stream << ",";
	if( m_TilingScale ) { m_TilingScale->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcFillAreaStyleTiles::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcFillAreaStyleTiles::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 3 ){ std::stringstream err; err << "Wrong parameter count for entity IfcFillAreaStyleTiles, expecting 3, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	readEntityReferenceList( args[0], m_TilingPattern, map, errorStream );
	readEntityReferenceList( args[1], m_Tiles, map, errorStream );
	m_TilingScale = IfcPositiveRatioMeasure::createObjectFromSTEP( args[2], map, errorStream );
}
void IFC4X3::IfcFillAreaStyleTiles::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcGeometricRepresentationItem::getAttributes( vec_attributes );
	shared_ptr<AttributeObjectVector> TilingPattern_vec_object( new AttributeObjectVector() );
	std::copy( m_TilingPattern.begin(), m_TilingPattern.end(), std::back_inserter( TilingPattern_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "TilingPattern", TilingPattern_vec_object ) );
	shared_ptr<AttributeObjectVector> Tiles_vec_object( new AttributeObjectVector() );
	std::copy( m_Tiles.begin(), m_Tiles.end(), std::back_inserter( Tiles_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "Tiles", Tiles_vec_object ) );
	vec_attributes.emplace_back( std::make_pair( "TilingScale", m_TilingScale ) );
}
void IFC4X3::IfcFillAreaStyleTiles::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcGeometricRepresentationItem::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcFillAreaStyleTiles::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcGeometricRepresentationItem::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcFillAreaStyleTiles::unlinkFromInverseCounterparts()
{
	IfcGeometricRepresentationItem::unlinkFromInverseCounterparts();
}