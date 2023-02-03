/* -*-c++-*- IfcQuery www.ifcquery.com
*
MIT License

Copyright (c) 2017 Fabian Gerold

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "IFCC_GeometryConverter.h"

#include <unordered_set>
#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/OpenMPIncludes.h>
#include <ifcpp/model/StatusCallback.h>
#include <ifcpp/IFC4/include/IfcCurtainWall.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcPropertySetDefinitionSet.h>
#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcRelContainedInSpatialStructure.h>
#include <ifcpp/IFC4/include/IfcRelDefinesByProperties.h>
#include <ifcpp/IFC4/include/IfcSpace.h>
#include <ifcpp/IFC4/include/IfcWindow.h>
#include <ifcpp/IFC4/include/IfcWall.h>
#include <ifcpp/IFC4/include/IfcDoor.h>
#include <ifcpp/IFC4/include/IfcLocalPlacement.h>

#include <carve/carve.hpp>

#include <ifcpp/geometry/Carve/IncludeCarveHeaders.h>
#include <ifcpp/geometry/Carve/GeometryInputData.h>
#include <ifcpp/geometry/Carve/CSG_Adapter.h>

#include "IFCC_RepresentationConverter.h"
#include "IFCC_Helper.h"


namespace IFCC {

GeometryConverter::GeometryConverter( shared_ptr<BuildingModel>& ifc_model )
{
	m_ifc_model = ifc_model;
	m_geom_settings = shared_ptr<GeometrySettings>( new GeometrySettings() );
	resetNumVerticesPerCircle();
	shared_ptr<UnitConverter>& unit_converter = m_ifc_model->getUnitConverter();
	m_representation_converter = shared_ptr<RepresentationConverter>( new RepresentationConverter( m_geom_settings, unit_converter ) );

	// redirect all messages to this->messageTarget
	m_ifc_model->setMessageTarget( this );
	m_representation_converter->setMessageTarget( this );
}

GeometryConverter::~GeometryConverter() {}

	// getters and setters
	shared_ptr<BuildingModel>& GeometryConverter::getBuildingModel() {
		return m_ifc_model;
	}

	shared_ptr<RepresentationConverter>& GeometryConverter::getRepresentationConverter() {
		return m_representation_converter;
	}

	shared_ptr<GeometrySettings>& GeometryConverter::getGeomSettings() {
		return m_geom_settings;
	}

	std::map<std::string, shared_ptr<ProductShapeData> >& GeometryConverter::getShapeInputData() {
		return m_product_shape_data;
	}

	std::map<std::string, shared_ptr<BuildingObject> >& GeometryConverter::getObjectsOutsideSpatialStructure() {
		return m_map_outside_spatial_structure;
	}

	void GeometryConverter::resetModel() {
		progressTextCallback( L"Unloading model, cleaning up memory..." );
		clearInputCache();
		m_recent_progress = 0.0;

		m_ifc_model->clearCache();
		m_ifc_model->clearIfcModel();
		progressTextCallback( L"Unloading model done" );
		progressValueCallback( 0.0, "parse" );

#ifdef _DEBUG
		GeomDebugDump::clearMeshsetDump();
#endif
	}

	void GeometryConverter::clearInputCache() {
		m_product_shape_data.clear();
		m_map_outside_spatial_structure.clear();
		m_representation_converter->clearCache();
		m_messages.clear();
	}

	void GeometryConverter::resetNumVerticesPerCircle() {
		m_geom_settings->resetNumVerticesPerCircle();
	}

	void GeometryConverter::setCsgEps(double eps) {
		m_csg_eps = eps;
	}

	void GeometryConverter::setModel( shared_ptr<BuildingModel> model ) {
		if( m_ifc_model ) {
			m_ifc_model->unsetMessageCallBack();
		}
		clearInputCache();
		m_ifc_model = model;
		m_representation_converter->clearCache();
		m_representation_converter->setUnitConverter( m_ifc_model->getUnitConverter() );
		m_ifc_model->setMessageTarget( this );
	}

	void GeometryConverter::resolveProjectStructure( shared_ptr<ProductShapeData>& product_data ) {
		if( !product_data ) {
			return;
		}
		if( product_data->m_ifc_object_definition.expired() ) {
			return;
		}

		shared_ptr<IfcObjectDefinition> ifc_object_def(product_data->m_ifc_object_definition);
		if (!ifc_object_def) {
			return;
		}

		product_data->m_added_to_spatial_structure = true;

		const std::vector<weak_ptr<IfcRelAggregates> >& vec_IsDecomposedBy = ifc_object_def->m_IsDecomposedBy_inverse;
		for( size_t ii = 0; ii < vec_IsDecomposedBy.size(); ++ii ) {
			const weak_ptr<IfcRelAggregates>& rel_aggregates_weak_ptr = vec_IsDecomposedBy[ii];
			if( rel_aggregates_weak_ptr.expired() ) {
				continue;
			}
			shared_ptr<IfcRelAggregates> rel_aggregates( rel_aggregates_weak_ptr );
			if( rel_aggregates ) {
				const std::vector<shared_ptr<IfcObjectDefinition> >& vec_related_objects = rel_aggregates->m_RelatedObjects;
				for( size_t jj = 0; jj < vec_related_objects.size(); ++jj ) {
					const shared_ptr<IfcObjectDefinition>& related_obj_def = vec_related_objects[jj];
					if( related_obj_def ) {
						std::string related_guid;
						if (related_obj_def->m_GlobalId) {
							std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
							related_guid = converterX.to_bytes(related_obj_def->m_GlobalId->m_value);
						}

						auto it_product_map = m_product_shape_data.find(related_guid);
						if( it_product_map != m_product_shape_data.end() ) {
							shared_ptr<ProductShapeData>& related_product_shape = it_product_map->second;
							if( related_product_shape ) {
								product_data->addChildProduct( related_product_shape, product_data );
								resolveProjectStructure( related_product_shape );
							}
						}
					}
				}
			}
		}

		shared_ptr<IfcSpatialStructureElement> spatial_ele = dynamic_pointer_cast<IfcSpatialStructureElement>(ifc_object_def);
		if( spatial_ele ) {
			const std::vector<weak_ptr<IfcRelContainedInSpatialStructure> >& vec_contains = spatial_ele->m_ContainsElements_inverse;
			for( size_t ii = 0; ii < vec_contains.size(); ++ii ) {
				const weak_ptr<IfcRelContainedInSpatialStructure>& rel_contained_weak_ptr = vec_contains[ii];
				if( rel_contained_weak_ptr.expired() ) {
					continue;
				}
				shared_ptr<IfcRelContainedInSpatialStructure> rel_contained( rel_contained_weak_ptr );
				if( rel_contained ) {
					const std::vector<shared_ptr<IfcProduct> >& vec_related_elements = rel_contained->m_RelatedElements;

					for( size_t jj = 0; jj < vec_related_elements.size(); ++jj ) {
						const shared_ptr<IfcProduct>& related_product = vec_related_elements[jj];
						if( related_product ) {
							std::string related_guid;
							if (related_product->m_GlobalId) {
								std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
								related_guid = converterX.to_bytes(related_product->m_GlobalId->m_value);
							}

							auto it_product_map = m_product_shape_data.find(related_guid);
							if( it_product_map != m_product_shape_data.end() ) {
								shared_ptr<ProductShapeData>& related_product_shape = it_product_map->second;
								if( related_product_shape ) {
									product_data->addChildProduct( related_product_shape, product_data );
									resolveProjectStructure( related_product_shape );
								}
							}
						}
					}
				}
			}
		}

		// TODO: handle IfcRelAssignsToProduct
	}

	void GeometryConverter::readAppearanceFromPropertySet( const shared_ptr<IfcPropertySet>& prop_set, shared_ptr<ProductShapeData>& product_shape ) {
		if( !prop_set ) {
			return;
		}
		for( auto& ifc_property : prop_set->m_HasProperties ) {
			if( !ifc_property ) {
				continue;
			}

			shared_ptr<IfcSimpleProperty> simple_property = dynamic_pointer_cast<IfcSimpleProperty>(ifc_property);
			if( simple_property ) {
				// ENTITY IfcSimpleProperty ABSTRACT SUPERTYPE OF(ONEOF( IfcPropertyBoundedValue, IfcPropertyEnumeratedValue, IfcPropertyListValue,
				// IfcPropertyReferenceValue, IfcPropertySingleValue, IfcPropertyTableValue))

				shared_ptr<IfcIdentifier> property_name = simple_property->m_Name;
				std::wstring name_str = property_name->m_value;
				if( name_str.compare( L"LayerName" ) == 0 ) {
					// TODO: implement layers
				}
				shared_ptr<IfcText> description = simple_property->m_Description;


				shared_ptr<IfcPropertySingleValue> property_single_value = dynamic_pointer_cast<IfcPropertySingleValue>(simple_property);
				if( property_single_value ) {
					//shared_ptr<IfcValue>& nominal_value = property_single_value->m_NominalValue;				//optional
					//shared_ptr<IfcUnit>& unit = property_single_value->m_Unit;						//optional

				}

				continue;
			}

			shared_ptr<IfcComplexProperty> complex_property = dynamic_pointer_cast<IfcComplexProperty>(ifc_property);
			if( complex_property ) {
				if( !complex_property->m_UsageName ) continue;
				if( complex_property->m_UsageName->m_value.compare( L"Color" ) == 0 ) {
					vec4 vec_color;
					m_representation_converter->getStylesConverter()->convertIfcComplexPropertyColor( complex_property, vec_color );
					shared_ptr<AppearanceData> appearance_data( new AppearanceData( -1 ) );
					if( !appearance_data ) {
						throw OutOfMemoryException( __FUNC__ );
					}
					appearance_data->m_apply_to_geometry_type = AppearanceData::GEOM_TYPE_ANY;
					appearance_data->m_color_ambient.setColor( vec_color );
					appearance_data->m_color_diffuse.setColor( vec_color );
					appearance_data->m_color_specular.setColor( vec_color );
					appearance_data->m_shininess = 35.f;
					product_shape->addAppearance( appearance_data );
				}
			}
		}
	}

	void GeometryConverter::resetIfcSiteLargeCoords(shared_ptr<IfcSite>& ifc_site) {
		if (!ifc_site) {
			return;
		}

		if (!ifc_site->m_ObjectPlacement) {
			return;
		}

		shared_ptr<IfcLocalPlacement> local_placement = dynamic_pointer_cast<IfcLocalPlacement>(ifc_site->m_ObjectPlacement);
		if (!local_placement) {
			return;
		}

		if (local_placement->m_RelativePlacement) {
			shared_ptr<IfcAxis2Placement3D> axis_placement = dynamic_pointer_cast<IfcAxis2Placement3D>(local_placement->m_RelativePlacement);
			if (axis_placement) {
				if (axis_placement->m_Location) {
					shared_ptr<IfcCartesianPoint> placement_location = dynamic_pointer_cast<IfcCartesianPoint>(axis_placement->m_Location);
					if (placement_location) {
						if (placement_location->m_Coordinates.size() > 2) {
							if (placement_location->m_Coordinates[0]) {
								if (placement_location->m_Coordinates[0]->m_value > 1000) {
									placement_location->m_Coordinates[0]->m_value = 0;
								}
							}

							if (placement_location->m_Coordinates[1]) {
								if (placement_location->m_Coordinates[1]->m_value > 1000) {
									placement_location->m_Coordinates[1]->m_value = 0;
								}
							}

							if (placement_location->m_Coordinates[2]) {
								if (placement_location->m_Coordinates[2]->m_value > 1000) {
									placement_location->m_Coordinates[2]->m_value = 0;
								}
							}
						}
					}
				}
			}
		}
	}

	/*\brief method convertGeometry: Creates geometry for Carve from previously loaded BuildingModel model.
	**/
	void GeometryConverter::convertGeometry(bool performSubtractOpenings, std::vector<ConvertError>& errors) {
		progressTextCallback( L"Creating geometry..." );
		progressValueCallback( 0, "geometry" );
		m_product_shape_data.clear();
		m_map_outside_spatial_structure.clear();
		m_representation_converter->clearCache();

		if( !m_ifc_model ) {
			return;
		}

		shared_ptr<ProductShapeData> ifc_project_data;
		std::vector<shared_ptr<IfcObjectDefinition> > vec_object_definitions;
		double length_to_meter_factor = 1.0;
		if( m_ifc_model->getUnitConverter() ) {
			length_to_meter_factor = m_ifc_model->getUnitConverter()->getLengthInMeterFactor();
		}
		carve::setEpsilon( m_csg_eps );

		const std::map<int, shared_ptr<BuildingEntity> >& map_entities = m_ifc_model->getMapIfcEntities();
		if (map_entities.size() > 0) {
			for (auto it = map_entities.begin(); it != map_entities.end(); ++it) {
				shared_ptr<BuildingEntity> obj = it->second;
				if (obj) {
					shared_ptr<IfcObjectDefinition> object_def = dynamic_pointer_cast<IfcObjectDefinition>(obj);
					if (object_def) {
						vec_object_definitions.push_back(object_def);

						shared_ptr<IfcSite> ifc_site = dynamic_pointer_cast<IfcSite>(object_def);
						if (ifc_site) {
							resetIfcSiteLargeCoords(ifc_site);
						}
					}
				}
			}
		}

		// create geometry for for each IfcProduct independently, spatial structure will be resolved later
		std::map<std::string, shared_ptr<ProductShapeData> >* map_products_ptr = &m_product_shape_data;
		const int num_object_definitions = (int)vec_object_definitions.size();

#ifdef ENABLE_OPENMP
		Mutex writelock_map;
		Mutex writelock_ifc_project;

#pragma omp parallel firstprivate(num_object_definitions) shared(map_products_ptr)
		{
			// time for one product may vary significantly, so schedule not so many
#pragma omp for schedule(dynamic,40)
#endif
			for( int i = 0; i < num_object_definitions; ++i ) {
				shared_ptr<IfcObjectDefinition> object_def = vec_object_definitions[i];
				BuildingElementTypes type = getObjectType(object_def);
				bool geometricObject = isConstructionType(type) || isOpeningType(type) || isConstructionSimilarType(type);

				const int entity_id = object_def->m_entity_id;
				std::string guid;
				std::wstring guid_wstr;
				if (object_def->m_GlobalId) {
					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
					guid_wstr = object_def->m_GlobalId->m_value;
					guid = converterX.to_bytes(guid_wstr);
				}

				shared_ptr<ProductShapeData> product_geom_input_data( new ProductShapeData(guid_wstr) );
				product_geom_input_data->m_ifc_object_definition = object_def;

				// TODO: check for equal product shapes: each representation and each item must be equal, also openings must be equal: m_HasOpenings_inverse
				std::stringstream thread_err;
				// check if the object_def is convertible into a IfcFeatureElementSubtraction object
				// this is the base class for all openenings
				// that means: this function checks if this is an opening
				if( performSubtractOpenings && !m_geom_settings->getRenderObjectFilter()(object_def) ) {
					// geometry will be created in method subtractOpenings
					continue;
				}
				else if( dynamic_pointer_cast<IfcProject>(object_def) ) {
#ifdef ENABLE_OPENMP
					ScopedLock scoped_lock( writelock_ifc_project );
#endif
					ifc_project_data = product_geom_input_data;
				}
				std::wstring nameOfSpace;
				if( dynamic_pointer_cast<IfcSpace>(object_def)) {
					nameOfSpace = dynamic_pointer_cast<IfcSpace>(object_def)->m_LongName->m_value;
					int i=0;
				}
				if( dynamic_pointer_cast<IfcWall>(object_def)) {
					nameOfSpace = dynamic_pointer_cast<IfcWall>(object_def)->m_Name->m_value;
					int i=0;
				}
				if( dynamic_pointer_cast<IfcDoor>(object_def)) {
					nameOfSpace = dynamic_pointer_cast<IfcDoor>(object_def)->m_Name->m_value;
					int i=0;
				}

				try {
					convertIfcProductShape( product_geom_input_data, performSubtractOpenings, errors );
				}
				catch( OutOfMemoryException& e ) {
					throw e;
				}
				catch( BuildingException& e ) {
					thread_err << e.what();
					errors.push_back({OT_GeometryConvert, -1, std::string(e.what()) + " in " + std::to_string(entity_id)});
				}
				catch( carve::exception& e ) {
					thread_err << e.str();
					errors.push_back({OT_GeometryConvert, -1, e.str() + " in " + std::to_string(entity_id)});
				}
				catch( std::exception& e ) {
					thread_err << e.what();
					errors.push_back({OT_GeometryConvert, -1, std::string(e.what()) + " in " + std::to_string(entity_id)});
				}
				catch( ... ) {
					thread_err << "undefined error, product id " << entity_id;
					errors.push_back({OT_GeometryConvert, -1, "undefined error, product id " + std::to_string(entity_id)});
				}

				{
#ifdef ENABLE_OPENMP
					ScopedLock scoped_lock( writelock_map );
#endif
					map_products_ptr->insert( std::make_pair( guid, product_geom_input_data ) );

					if( thread_err.tellp() > 0 ) {
						messageCallback( thread_err.str().c_str(), StatusCallback::MESSAGE_TYPE_ERROR, __FUNC__ );
					}
				}

				// progress callback
				double progress = (double)i / (double)num_object_definitions;
				if( progress - m_recent_progress > 0.02 ) {

#ifdef ENABLE_OPENMP
					if( omp_get_thread_num() == 0 )
#endif
					{
						// leave 10% of progress to openscenegraph internals
						progressValueCallback( progress*0.9, "geometry" );
						m_recent_progress = progress;
					}
				}
			}
#ifdef ENABLE_OPENMP
		} // implicit barrier
#endif

		// subtract openings in related objects, such as IFCBUILDINGELEMENTPART connected to a window through IFCRELAGGREGATES
		for( auto it = map_products_ptr->begin(); it != map_products_ptr->end(); ++it ) {
			shared_ptr<ProductShapeData> product_geom_input_data = it->second;
			try {
				if(performSubtractOpenings) {
					subtractOpeningsInRelatedObjects(product_geom_input_data, errors);
				}
			}
			catch( OutOfMemoryException& e ) {
				throw e;
			}
			catch( BuildingException& e ) {
				messageCallback(e.what(), StatusCallback::MESSAGE_TYPE_ERROR, "");
			}
			catch( carve::exception& e ) {
				messageCallback(e.str(), StatusCallback::MESSAGE_TYPE_ERROR, "");
			}
			catch( std::exception& e ) {
				messageCallback(e.what(), StatusCallback::MESSAGE_TYPE_ERROR, "");
			}
			catch( ... ) {
				messageCallback("undefined error", StatusCallback::MESSAGE_TYPE_ERROR, __FUNC__);
			}
		}

		try {
			// now resolve spatial structure
			if( ifc_project_data ) {
				resolveProjectStructure( ifc_project_data );
			}

			// check if there are entities that are not in spatial structure
			for( auto it_product_shapes = m_product_shape_data.begin(); it_product_shapes != m_product_shape_data.end(); ++it_product_shapes ) {
				shared_ptr<ProductShapeData> product_shape = it_product_shapes->second;
				if( !product_shape ) {
					continue;
				}

				if( !product_shape->m_added_to_spatial_structure ) {
					if( !product_shape->m_ifc_object_definition.expired() ) {
						shared_ptr<IfcObjectDefinition> ifc_object_def( product_shape->m_ifc_object_definition );
						shared_ptr<IfcFeatureElementSubtraction> opening = dynamic_pointer_cast<IfcFeatureElementSubtraction>(ifc_object_def);
						if( !m_geom_settings->getRenderObjectFilter()(ifc_object_def) ) {
							continue;
						}
						std::string guid;
						if (ifc_object_def->m_GlobalId) {
							std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
							guid = converterX.to_bytes(ifc_object_def->m_GlobalId->m_value);
						}
						m_map_outside_spatial_structure[guid] = ifc_object_def;
					}
				}
			}
		}
		catch( OutOfMemoryException& e ) {
			throw e;
		}
		catch( BuildingException& e ) {
			messageCallback( e.what(), StatusCallback::MESSAGE_TYPE_ERROR, "" );
		}
		catch( std::exception& e ) {
			messageCallback( e.what(), StatusCallback::MESSAGE_TYPE_ERROR, "" );
		}
		catch( ... ) {
			messageCallback( "undefined error", StatusCallback::MESSAGE_TYPE_ERROR, __FUNC__ );
		}

		m_representation_converter->getProfileCache()->clearProfileCache();
		progressTextCallback( L"Loading file done" );
		progressValueCallback( 1.0, "geometry" );
	}

	void GeometryConverter::addVector3D(const vec3& point, std::vector<float>& target_array) {
		bool m_roundCoords = false;
		if (m_roundCoords) {
			target_array.push_back(round(point.x*10000)*0.0001);
			target_array.push_back(round(point.y*10000)*0.0001);
			target_array.push_back(round(point.z*10000)*0.0001);
		}
		else {
			target_array.push_back(point.x);
			target_array.push_back(point.y);
			target_array.push_back(point.z);
		}
	}

	//\brief method convertIfcProduct: Creates geometry objects (meshset with connected vertex-edge-face graph) from an IfcProduct object
	// caution: when using OpenMP, this method runs in parallel threads, so every write access to member variables needs a write lock
	void GeometryConverter::convertIfcProductShape( shared_ptr<ProductShapeData>& product_shape, bool performSubtractOpenings,
													std::vector<ConvertError>& errors) {
		if( product_shape->m_ifc_object_definition.expired() ) {
			return;
		}

		shared_ptr<IfcObjectDefinition> ifc_object_def(product_shape->m_ifc_object_definition);
		shared_ptr<IfcProduct> ifc_product = dynamic_pointer_cast<IfcProduct>(ifc_object_def);
		if (!ifc_product)
		{
			return;
		}

		double length_factor = 1.0;
		if (m_ifc_model)
		{
			if (m_ifc_model->getUnitConverter())
			{
				length_factor = m_ifc_model->getUnitConverter()->getLengthInMeterFactor();
			}
		}

		if( !ifc_product->m_Representation )
		{
			return;
		}

		// evaluate IFC geometry
		shared_ptr<IfcProductRepresentation>& product_representation = ifc_product->m_Representation;
		std::vector<shared_ptr<IfcRepresentation> >& vec_representations = product_representation->m_Representations;
		for( size_t i_representations = 0; i_representations < vec_representations.size(); ++i_representations )
		{
			const shared_ptr<IfcRepresentation>& representation = vec_representations[i_representations];
			if( !representation )
			{
				continue;
			}

			try
			{
				shared_ptr<RepresentationData> representation_data( new RepresentationData() );
				bool res = m_representation_converter->convertIfcRepresentation( representation, representation_data, errors );
				product_shape->m_vec_representations.push_back( representation_data );
				representation_data->m_parent_product = product_shape;
			}
			catch( OutOfMemoryException& e )
			{
				throw e;
			}
			catch( BuildingException& e )
			{
				messageCallback( e.what(), StatusCallback::MESSAGE_TYPE_ERROR, "" );
			}
			catch( std::exception& e )
			{
				messageCallback( e.what(), StatusCallback::MESSAGE_TYPE_ERROR, "" );
			}
		}

		// IfcProduct has an ObjectPlacement that can be local or global
		product_shape->m_object_placement = ifc_product->m_ObjectPlacement;
		if( ifc_product->m_ObjectPlacement )
		{
			// IfcPlacement2Matrix follows related placements in case of local coordinate systems
			std::unordered_set<IfcObjectPlacement*> placement_already_applied;
			m_representation_converter->getPlacementConverter()->convertIfcObjectPlacement( ifc_product->m_ObjectPlacement, product_shape, placement_already_applied, false );
		}


		std::vector<shared_ptr<ProductShapeData> > vec_opening_data;
		const shared_ptr<IfcElement> ifc_element = dynamic_pointer_cast<IfcElement>(ifc_product);
		if( ifc_element )
		{
			// handle openings
			if(performSubtractOpenings)
				m_representation_converter->subtractOpenings(ifc_element, product_shape, errors);

			// handle styles on IfcElement level
			std::vector<shared_ptr<AppearanceData> > vec_apperances;
			StylesConverter::convertElementStyle(ifc_element, vec_apperances);
			for (auto appearance_data : vec_apperances)
			{
				product_shape->addAppearance(appearance_data);
			}
		}

		// Fetch the IFCProduct relationships
		if( ifc_product->m_IsDefinedBy_inverse.size() > 0 )
		{
			std::vector<weak_ptr<IfcRelDefinesByProperties> >& vec_IsDefinedBy_inverse = ifc_product->m_IsDefinedBy_inverse;
			for( size_t i = 0; i < vec_IsDefinedBy_inverse.size(); ++i )
			{
				shared_ptr<IfcRelDefinesByProperties> rel_def( vec_IsDefinedBy_inverse[i] );
				shared_ptr<IfcPropertySetDefinitionSelect> relating_property_definition_select = rel_def->m_RelatingPropertyDefinition;
				if( relating_property_definition_select )
				{
					// TYPE IfcPropertySetDefinitionSelect = SELECT	(IfcPropertySetDefinition	,IfcPropertySetDefinitionSet);
					shared_ptr<IfcPropertySetDefinition> property_set_def = dynamic_pointer_cast<IfcPropertySetDefinition>(relating_property_definition_select);
					if( property_set_def )
					{
						shared_ptr<IfcPropertySet> property_set = dynamic_pointer_cast<IfcPropertySet>(property_set_def);
						if( property_set )
						{
							readAppearanceFromPropertySet( property_set, product_shape );
						}
						continue;
					}

					shared_ptr<IfcPropertySetDefinitionSet> property_set_def_set = dynamic_pointer_cast<IfcPropertySetDefinitionSet>(relating_property_definition_select);
					if( property_set_def_set )
					{
						std::vector<shared_ptr<IfcPropertySetDefinition> >& vec_propterty_set_def = property_set_def_set->m_vec;
						std::vector<shared_ptr<IfcPropertySetDefinition> >::iterator it_property_set_def;
						for( it_property_set_def = vec_propterty_set_def.begin(); it_property_set_def != vec_propterty_set_def.end(); ++it_property_set_def )
						{
							shared_ptr<IfcPropertySetDefinition> property_set_def2 = (*it_property_set_def);
							if( property_set_def2 )
							{
								shared_ptr<IfcPropertySet> property_set = dynamic_pointer_cast<IfcPropertySet>(property_set_def2);
								if( property_set )
								{
									readAppearanceFromPropertySet( property_set, product_shape );
								}
							}
						}
						continue;
					}
				}
			}
		}
	}

	bool GeometryConverter::hasRelatedOpenings(shared_ptr<ProductShapeData>& product_shape) {
		if (product_shape->m_ifc_object_definition.expired()) {
			return false;
		}

		shared_ptr<IfcObjectDefinition> ifc_object_def(product_shape->m_ifc_object_definition);
		shared_ptr<IfcProduct> ifc_product = dynamic_pointer_cast<IfcProduct>(ifc_object_def);
		if (!ifc_product) {
			return false;
		}

		shared_ptr<IfcElement> ifc_element = dynamic_pointer_cast<IfcElement>(ifc_product);
		if (!ifc_element) {
			return false;
		}

		if (ifc_element->m_HasOpenings_inverse.size() == 0) {
			return false;
		}

		// collect aggregated objects
		const std::vector<weak_ptr<IfcRelAggregates> >& vec_decomposed_by = ifc_object_def->m_IsDecomposedBy_inverse;
		if (vec_decomposed_by.size() > 0) {
			return true;
		}

		const std::vector<weak_ptr<IfcRelAggregates> >& vec_decomposes = ifc_object_def->m_Decomposes_inverse;
		if (vec_decomposes.size() > 0) {
			return true;
		}
		return false;
	}

	void GeometryConverter::subtractOpeningsInRelatedObjects(shared_ptr<ProductShapeData>& product_shape, std::vector<ConvertError>& errors) {
		if( product_shape->m_ifc_object_definition.expired() ) {
			return;
		}

		shared_ptr<IfcObjectDefinition> ifc_object_def(product_shape->m_ifc_object_definition);
		shared_ptr<IfcProduct> ifc_product = dynamic_pointer_cast<IfcProduct>(ifc_object_def);
		if (!ifc_product) {
			return;
		}

		shared_ptr<IfcElement> ifc_element = dynamic_pointer_cast<IfcElement>(ifc_product);
		if( !ifc_element ) {
			return;
		}

		if( ifc_element->m_HasOpenings_inverse.size() == 0 ) {
			return;
		}

		// collect aggregated objects
		const std::vector<weak_ptr<IfcRelAggregates> >& vec_decomposed_by = ifc_element->m_IsDecomposedBy_inverse;

		for( auto& decomposed_by : vec_decomposed_by ) {
			if( decomposed_by.expired() ) {
				continue;
			}
			shared_ptr<IfcRelAggregates> decomposed_by_aggregates(decomposed_by);
			std::vector<shared_ptr<IfcObjectDefinition> >& vec_related_objects = decomposed_by_aggregates->m_RelatedObjects;
			for( auto& related_object : vec_related_objects ) {
				if( !related_object ) {
					continue;
				}

				std::string guid;
				if (related_object->m_GlobalId) {
					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
					guid = converterX.to_bytes(related_object->m_GlobalId->m_value);

					auto it_find_related_shape = m_product_shape_data.find(guid);
					if( it_find_related_shape != m_product_shape_data.end() ) {
						shared_ptr<ProductShapeData>& related_product_shape = it_find_related_shape->second;
						bool res = m_representation_converter->subtractOpenings(ifc_element, related_product_shape, errors);
					}
				}
			}
		}
	}

	void GeometryConverter::messageTarget( void* ptr, shared_ptr<StatusCallback::Message> m ) {
		GeometryConverter* myself = (GeometryConverter*)ptr;
		if( myself ) {
			if( m->m_entity ) {
#ifdef ENABLE_OPENMP
				ScopedLock lock( myself->m_writelock_messages );
#endif
				// make sure that the same message for one entity does not appear several times
				const int entity_id = m->m_entity->m_entity_id;

				auto it = myself->m_messages.find( entity_id );
				if( it != myself->m_messages.end() ) {
					std::vector<shared_ptr<StatusCallback::Message> >& vec_message_for_entity = it->second;
					for( size_t i = 0; i < vec_message_for_entity.size(); ++i )
					{
						shared_ptr<StatusCallback::Message>& existing_message = vec_message_for_entity[i];
						if( existing_message->m_message_text.compare( m->m_message_text ) == 0 )
						{
							// same message for same entity is already there, so ignore message
							return;
						}
					}
					vec_message_for_entity.push_back( m );
				}
				else {
					std::vector<shared_ptr<StatusCallback::Message> >& vec = myself->m_messages.insert( std::make_pair( entity_id, std::vector<shared_ptr<StatusCallback::Message> >() ) ).first->second;
					vec.push_back( m );
				}
			}

			myself->messageCallback( m );
		}
	}

} // end namespace

