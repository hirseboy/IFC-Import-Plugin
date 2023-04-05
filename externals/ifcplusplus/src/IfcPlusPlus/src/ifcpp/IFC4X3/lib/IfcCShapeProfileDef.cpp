/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcAxis2Placement2D.h"
#include "ifcpp/IFC4X3/include/IfcCShapeProfileDef.h"
#include "ifcpp/IFC4X3/include/IfcExternalReferenceRelationship.h"
#include "ifcpp/IFC4X3/include/IfcLabel.h"
#include "ifcpp/IFC4X3/include/IfcNonNegativeLengthMeasure.h"
#include "ifcpp/IFC4X3/include/IfcPositiveLengthMeasure.h"
#include "ifcpp/IFC4X3/include/IfcProfileProperties.h"
#include "ifcpp/IFC4X3/include/IfcProfileTypeEnum.h"

// ENTITY IfcCShapeProfileDef 
IFC4X3::IfcCShapeProfileDef::IfcCShapeProfileDef( int tag ) { m_tag = tag; }
void IFC4X3::IfcCShapeProfileDef::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCCSHAPEPROFILEDEF" << "(";
	if( m_ProfileType ) { m_ProfileType->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_ProfileName ) { m_ProfileName->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Position ) { stream << "#" << m_Position->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Depth ) { m_Depth->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Width ) { m_Width->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_WallThickness ) { m_WallThickness->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_Girth ) { m_Girth->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_InternalFilletRadius ) { m_InternalFilletRadius->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcCShapeProfileDef::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcCShapeProfileDef::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 8 ){ std::stringstream err; err << "Wrong parameter count for entity IfcCShapeProfileDef, expecting 8, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_ProfileType = IfcProfileTypeEnum::createObjectFromSTEP( args[0], map, errorStream );
	m_ProfileName = IfcLabel::createObjectFromSTEP( args[1], map, errorStream );
	readEntityReference( args[2], m_Position, map, errorStream );
	m_Depth = IfcPositiveLengthMeasure::createObjectFromSTEP( args[3], map, errorStream );
	m_Width = IfcPositiveLengthMeasure::createObjectFromSTEP( args[4], map, errorStream );
	m_WallThickness = IfcPositiveLengthMeasure::createObjectFromSTEP( args[5], map, errorStream );
	m_Girth = IfcPositiveLengthMeasure::createObjectFromSTEP( args[6], map, errorStream );
	m_InternalFilletRadius = IfcNonNegativeLengthMeasure::createObjectFromSTEP( args[7], map, errorStream );
}
void IFC4X3::IfcCShapeProfileDef::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcParameterizedProfileDef::getAttributes( vec_attributes );
	vec_attributes.emplace_back( std::make_pair( "Depth", m_Depth ) );
	vec_attributes.emplace_back( std::make_pair( "Width", m_Width ) );
	vec_attributes.emplace_back( std::make_pair( "WallThickness", m_WallThickness ) );
	vec_attributes.emplace_back( std::make_pair( "Girth", m_Girth ) );
	vec_attributes.emplace_back( std::make_pair( "InternalFilletRadius", m_InternalFilletRadius ) );
}
void IFC4X3::IfcCShapeProfileDef::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcParameterizedProfileDef::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcCShapeProfileDef::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcParameterizedProfileDef::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcCShapeProfileDef::unlinkFromInverseCounterparts()
{
	IfcParameterizedProfileDef::unlinkFromInverseCounterparts();
}
