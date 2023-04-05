/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcPlacement.h"
#include "ifcpp/IFC4X3/include/IfcPoint.h"
#include "ifcpp/IFC4X3/include/IfcPresentationLayerAssignment.h"
#include "ifcpp/IFC4X3/include/IfcStyledItem.h"

// ENTITY IfcPlacement 
IFC4X3::IfcPlacement::IfcPlacement( int tag ) { m_tag = tag; }
void IFC4X3::IfcPlacement::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCPLACEMENT" << "(";
	if( m_Location ) { stream << "#" << m_Location->m_tag; } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcPlacement::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcPlacement::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 1 ){ std::stringstream err; err << "Wrong parameter count for entity IfcPlacement, expecting 1, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	readEntityReference( args[0], m_Location, map, errorStream );
}
void IFC4X3::IfcPlacement::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcGeometricRepresentationItem::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "Location", m_Location ) );
}
void IFC4X3::IfcPlacement::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcGeometricRepresentationItem::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcPlacement::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcGeometricRepresentationItem::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcPlacement::unlinkFromInverseCounterparts()
{
	IfcGeometricRepresentationItem::unlinkFromInverseCounterparts();
}
