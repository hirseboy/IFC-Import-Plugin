/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcBSplineSurfaceForm.h"
#include "ifcpp/IFC4X3/include/IfcBSplineSurfaceWithKnots.h"
#include "ifcpp/IFC4X3/include/IfcCartesianPoint.h"
#include "ifcpp/IFC4X3/include/IfcInteger.h"
#include "ifcpp/IFC4X3/include/IfcKnotType.h"
#include "ifcpp/IFC4X3/include/IfcLogical.h"
#include "ifcpp/IFC4X3/include/IfcParameterValue.h"
#include "ifcpp/IFC4X3/include/IfcPresentationLayerAssignment.h"
#include "ifcpp/IFC4X3/include/IfcStyledItem.h"

// ENTITY IfcBSplineSurfaceWithKnots 
IFC4X3::IfcBSplineSurfaceWithKnots::IfcBSplineSurfaceWithKnots( int tag ) { m_tag = tag; }
void IFC4X3::IfcBSplineSurfaceWithKnots::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCBSPLINESURFACEWITHKNOTS" << "(";
	if( m_UDegree ) { m_UDegree->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_VDegree ) { m_VDegree->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	writeEntityList2D( stream, m_ControlPointsList );
	stream << ",";
	if( m_SurfaceForm ) { m_SurfaceForm->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_UClosed ) { m_UClosed->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_VClosed ) { m_VClosed->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	if( m_SelfIntersect ) { m_SelfIntersect->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	writeTypeOfIntList( stream, m_UMultiplicities, false );
	stream << ",";
	writeTypeOfIntList( stream, m_VMultiplicities, false );
	stream << ",";
	writeTypeOfRealList( stream, m_UKnots, false );
	stream << ",";
	writeTypeOfRealList( stream, m_VKnots, false );
	stream << ",";
	if( m_KnotSpec ) { m_KnotSpec->getStepParameter( stream ); } else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcBSplineSurfaceWithKnots::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcBSplineSurfaceWithKnots::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 12 ){ std::stringstream err; err << "Wrong parameter count for entity IfcBSplineSurfaceWithKnots, expecting 12, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	m_UDegree = IfcInteger::createObjectFromSTEP( args[0], map, errorStream );
	m_VDegree = IfcInteger::createObjectFromSTEP( args[1], map, errorStream );
	readEntityReferenceList2D( args[2], m_ControlPointsList, map, errorStream );
	m_SurfaceForm = IfcBSplineSurfaceForm::createObjectFromSTEP( args[3], map, errorStream );
	m_UClosed = IfcLogical::createObjectFromSTEP( args[4], map, errorStream );
	m_VClosed = IfcLogical::createObjectFromSTEP( args[5], map, errorStream );
	m_SelfIntersect = IfcLogical::createObjectFromSTEP( args[6], map, errorStream );
	readTypeOfIntegerList( args[7], m_UMultiplicities );
	readTypeOfIntegerList( args[8], m_VMultiplicities );
	readTypeOfRealList( args[9], m_UKnots );
	readTypeOfRealList( args[10], m_VKnots );
	m_KnotSpec = IfcKnotType::createObjectFromSTEP( args[11], map, errorStream );
}
void IFC4X3::IfcBSplineSurfaceWithKnots::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcBSplineSurface::getAttributes( vec_attributes );
	shared_ptr<AttributeObjectVector> UMultiplicities_vec_object( new AttributeObjectVector() );
	std::copy( m_UMultiplicities.begin(), m_UMultiplicities.end(), std::back_inserter( UMultiplicities_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "UMultiplicities", UMultiplicities_vec_object ) );
	shared_ptr<AttributeObjectVector> VMultiplicities_vec_object( new AttributeObjectVector() );
	std::copy( m_VMultiplicities.begin(), m_VMultiplicities.end(), std::back_inserter( VMultiplicities_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "VMultiplicities", VMultiplicities_vec_object ) );
	shared_ptr<AttributeObjectVector> UKnots_vec_object( new AttributeObjectVector() );
	std::copy( m_UKnots.begin(), m_UKnots.end(), std::back_inserter( UKnots_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "UKnots", UKnots_vec_object ) );
	shared_ptr<AttributeObjectVector> VKnots_vec_object( new AttributeObjectVector() );
	std::copy( m_VKnots.begin(), m_VKnots.end(), std::back_inserter( VKnots_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "VKnots", VKnots_vec_object ) );
	vec_attributes.emplace_back( std::make_pair( "KnotSpec", m_KnotSpec ) );
}
void IFC4X3::IfcBSplineSurfaceWithKnots::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcBSplineSurface::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcBSplineSurfaceWithKnots::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcBSplineSurface::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcBSplineSurfaceWithKnots::unlinkFromInverseCounterparts()
{
	IfcBSplineSurface::unlinkFromInverseCounterparts();
}
