/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcBoolean.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcTextFontSelect.h"
#include "ifcpp/IFC4X3/include/IfcTextStyle.h"
#include "ifcpp/IFC4X3/include/IfcTextStyleForDefinedFont.h"
#include "ifcpp/IFC4X3/include/IfcTextStyleTextModel.h"

// ENTITY IfcTextStyle 
IFC4X3::IfcTextStyle::IfcTextStyle( int tag ) { m_tag = tag; }
void IFC4X3::IfcTextStyle::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCTEXTSTYLE" << "(";
	if( m_Name ) { m_Name->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_TextCharacterAppearance ) { stream << "#" << m_TextCharacterAppearance->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_TextStyle ) { stream << "#" << m_TextStyle->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_TextFontStyle ) { m_TextFontStyle->getStepParameter( stream, true ); } else { stream << "$" ; }
	stream << ",";
	if( m_ModelOrDraughting ) { m_ModelOrDraughting->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcTextStyle::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcTextStyle::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 5 ){ std::stringstream err; err << "Wrong parameter count for entity IfcTextStyle, expecting 5, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_Name = IfcLabel::createObjectFromSTEP( args[0], map, errorStream );
	readEntityReference( args[1], m_TextCharacterAppearance, map, errorStream );
	readEntityReference( args[2], m_TextStyle, map, errorStream );
	m_TextFontStyle = IfcTextFontSelect::createObjectFromSTEP( args[3], map, errorStream );
	m_ModelOrDraughting = IfcBoolean::createObjectFromSTEP( args[4], map, errorStream );
}
void IFC4X3::IfcTextStyle::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcPresentationStyle::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "TextCharacterAppearance", m_TextCharacterAppearance ) );
	vec_attributes.emplace_back( std::make_pair( "TextStyle", m_TextStyle ) );
	vec_attributes.emplace_back( std::make_pair( "TextFontStyle", m_TextFontStyle ) );
	vec_attributes.emplace_back( std::make_pair( "ModelOrDraughting", m_ModelOrDraughting ) );
}
void IFC4X3::IfcTextStyle::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcPresentationStyle::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcTextStyle::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcPresentationStyle::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcTextStyle::unlinkFromInverseCounterparts()
{
	IfcPresentationStyle::unlinkFromInverseCounterparts();
}
