/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/AttributeObject.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/model/BuildingGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4X3/include/IfcBoolean.h"
#include "ifcpp/IFC4X3/include/IfcCartesianPointList3D.h"
#include "ifcpp/IFC4X3/include/IfcIndexedColourMap.h"
#include "ifcpp/IFC4X3/include/IfcIndexedTextureMap.h"
#include "ifcpp/IFC4X3/include/IfcParameterValue.h"
#include "ifcpp/IFC4X3/include/IfcPositiveInteger.h"
#include "ifcpp/IFC4X3/include/IfcPresentationLayerAssignment.h"
#include "ifcpp/IFC4X3/include/IfcStyledItem.h"
#include "ifcpp/IFC4X3/include/IfcTriangulatedFaceSet.h"

// ENTITY IfcTriangulatedFaceSet 
IFC4X3::IfcTriangulatedFaceSet::IfcTriangulatedFaceSet( int tag ) { m_tag = tag; }
void IFC4X3::IfcTriangulatedFaceSet::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_tag << "= IFCTRIANGULATEDFACESET" << "(";
	if( m_Coordinates ) { stream << "#" << m_Coordinates->m_tag; } else { stream << "$"; }
	stream << ",";
	if( m_Closed ) { m_Closed->getStepParameter( stream ); } else { stream << "$"; }
	stream << ",";
	writeTypeOfRealList2D( stream, m_Normals, true );
	stream << ",";
	writeTypeOfIntList2D( stream, m_CoordIndex, false );
	stream << ",";
	if( m_PnIndex.size() > 0 )
	{
		stream << "(";
		for( size_t ii = 0; ii < m_PnIndex.size(); ++ii )
		{
			if( ii > 0 )
			{
				stream << ",";
			}
			const shared_ptr<IfcPositiveInteger>& type_object = m_PnIndex[ii];
			if( type_object )
			{
				type_object->getStepParameter( stream, false );
			}
			else
			{
				stream << "$";
			}
		}
		stream << ")";
	}
	else { stream << "$"; }
	stream << ");";
}
void IFC4X3::IfcTriangulatedFaceSet::getStepParameter( std::stringstream& stream, bool /*is_select_type*/ ) const { stream << "#" << m_tag; }
void IFC4X3::IfcTriangulatedFaceSet::readStepArguments( const std::vector<std::string>& args, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	const size_t num_args = args.size();
	if( num_args != 5 ){ std::stringstream err; err << "Wrong parameter count for entity IfcTriangulatedFaceSet, expecting 5, having " << num_args << ". Entity ID: " << m_tag << std::endl; throw BuildingException( err.str().c_str() ); }
	readEntityReference( args[0], m_Coordinates, map, errorStream );
	m_Closed = IfcBoolean::createObjectFromSTEP( args[1], map, errorStream );
	readTypeOfRealList2D( args[2], m_Normals );
	readTypeOfIntegerList2D( args[3], m_CoordIndex );
	readTypeOfIntegerList( args[4], m_PnIndex );
}
void IFC4X3::IfcTriangulatedFaceSet::getAttributes( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes ) const
{
	IFC4X3::IfcTessellatedFaceSet::getAttributes( vec_attributes );
	shared_ptr<AttributeObjectVector> Normals_vector( new AttributeObjectVector() );
	vec_attributes.emplace_back( std::make_pair( "Normals", Normals_vector ) );
	for( size_t ii=0; ii<m_Normals.size(); ++ii )
	{
		const std::vector<shared_ptr<IfcParameterValue> >& vec_ii = m_Normals[ii];
		shared_ptr<AttributeObjectVector> inner_vector( new AttributeObjectVector() );
		Normals_vector->m_vec.push_back( inner_vector );
		std::copy(vec_ii.begin(), vec_ii.end(), std::back_inserter(inner_vector->m_vec));
	}
	shared_ptr<AttributeObjectVector> CoordIndex_vector( new AttributeObjectVector() );
	vec_attributes.emplace_back( std::make_pair( "CoordIndex", CoordIndex_vector ) );
	for( size_t ii=0; ii<m_CoordIndex.size(); ++ii )
	{
		const std::vector<shared_ptr<IfcPositiveInteger> >& vec_ii = m_CoordIndex[ii];
		shared_ptr<AttributeObjectVector> inner_vector( new AttributeObjectVector() );
		CoordIndex_vector->m_vec.push_back( inner_vector );
		std::copy(vec_ii.begin(), vec_ii.end(), std::back_inserter(inner_vector->m_vec));
	}
	shared_ptr<AttributeObjectVector> PnIndex_vec_object( new AttributeObjectVector() );
	std::copy( m_PnIndex.begin(), m_PnIndex.end(), std::back_inserter( PnIndex_vec_object->m_vec ) );
	vec_attributes.emplace_back( std::make_pair( "PnIndex", PnIndex_vec_object ) );
}
void IFC4X3::IfcTriangulatedFaceSet::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<BuildingObject> > >& vec_attributes_inverse ) const
{
	IFC4X3::IfcTessellatedFaceSet::getAttributesInverse( vec_attributes_inverse );
}
void IFC4X3::IfcTriangulatedFaceSet::setInverseCounterparts( shared_ptr<BuildingEntity> ptr_self_entity )
{
	IfcTessellatedFaceSet::setInverseCounterparts( ptr_self_entity );
}
void IFC4X3::IfcTriangulatedFaceSet::unlinkFromInverseCounterparts()
{
	IfcTessellatedFaceSet::unlinkFromInverseCounterparts();
}
