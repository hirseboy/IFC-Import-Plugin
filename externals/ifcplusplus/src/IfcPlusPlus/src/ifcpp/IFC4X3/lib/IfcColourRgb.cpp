/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcColourRgb.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcNormalisedRatioMeasure.h"

// ENTITY IfcColourRgb 
IFC4X3::IfcColourRgb::IfcColourRgb( int tag ) { m_tag = tag; }
void IFC4X3::IfcColourRgb::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCCOLOURRGB" << "(";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Red ) { m_Red->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Green ) { m_Green->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Blue ) { m_Blue->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcColourRgb::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcColourRgb::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 4 ){ std::stringstream err; err << "Wrong parameter count for entity IfcColourRgb, expecting 4, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_Name = IfcLabel::createObjectFromSTEP( args[0], map, errorStream );
	m_Red = IfcNormalisedRatioMeasure::createObjectFromSTEP( args[1], map, errorStream );
	m_Green = IfcNormalisedRatioMeasure::createObjectFromSTEP( args[2], map, errorStream );
	m_Blue = IfcNormalisedRatioMeasure::createObjectFromSTEP( args[3], map, errorStream );
}
void IFC4X3::IfcColourRgb::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcColourSpecification::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "Red", m_Red ) );
	vec_attributes.emplace_back( std::make_pair( "Green", m_Green ) );
	vec_attributes.emplace_back( std::make_pair( "Blue", m_Blue ) );
}
void IFC4X3::IfcColourRgb::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcColourSpecification::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcColourRgb::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcColourSpecification::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcColourRgb::unlinkFromInverseCounterparts()
{
	IfcColourSpecification::unlinkFromInverseCounterparts();
}
