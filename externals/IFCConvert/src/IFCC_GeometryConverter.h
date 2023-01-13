#ifndef IFCC_GeometryConverterH
#define IFCC_GeometryConverterH

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


#include <unordered_set>

#include <ifcpp/IFC4/include/IfcWindow.h>
#include <ifcpp/IFC4/include/IfcSite.h>

#include <ifcpp/model/BuildingModel.h>

#include "IFCC_RepresentationConverter.h"
#include "IFCC_Types.h"

namespace IFCC {

/*! Class for IFC objects into global geometry. Create shape data for all building objects.*/
class GeometryConverter : public StatusCallback {
public:
	GeometryConverter( shared_ptr<BuildingModel>& ifc_model );
	virtual ~GeometryConverter();

	// getters and setters
	shared_ptr<BuildingModel>&						getBuildingModel();
	shared_ptr<RepresentationConverter>&			getRepresentationConverter();
	shared_ptr<GeometrySettings>&					getGeomSettings();
	std::map<std::string, shared_ptr<ProductShapeData> >&	getShapeInputData();
	std::map<std::string, shared_ptr<BuildingObject> >&		getObjectsOutsideSpatialStructure();

	void resetModel();

	void clearInputCache();

	void resetNumVerticesPerCircle();

	void setCsgEps(double eps);

	void setModel( shared_ptr<BuildingModel> model );

	void resolveProjectStructure( shared_ptr<ProductShapeData>& product_data );

	void readAppearanceFromPropertySet( const shared_ptr<IfcPropertySet>& prop_set, shared_ptr<ProductShapeData>& product_shape );

	void resetIfcSiteLargeCoords(shared_ptr<IfcSite>& ifc_site);

	/*\brief method convertGeometry: Creates geometry for Carve from previously loaded BuildingModel model.
	**/
	void convertGeometry(bool performSubtractOpenings, std::vector<ConvertError>& errors);

	void addVector3D(const vec3& point, std::vector<float>& target_array);

	//\brief method convertIfcProduct: Creates geometry objects (meshset with connected vertex-edge-face graph) from an IfcProduct object
	// caution: when using OpenMP, this method runs in parallel threads, so every write access to member variables needs a write lock
	void convertIfcProductShape( shared_ptr<ProductShapeData>& product_shape, bool performSubtractOpenings, std::vector<ConvertError>& errors );

	bool hasRelatedOpenings(shared_ptr<ProductShapeData>& product_shape);

	void subtractOpeningsInRelatedObjects(shared_ptr<ProductShapeData>& product_shape, std::vector<ConvertError>& errors);

	virtual void messageTarget( void* ptr, shared_ptr<StatusCallback::Message> m );

protected:
	shared_ptr<BuildingModel>											m_ifc_model;
	shared_ptr<GeometrySettings>										m_geom_settings;
	shared_ptr<RepresentationConverter>									m_representation_converter;

	std::map<std::string, shared_ptr<ProductShapeData> >				m_product_shape_data;
	std::map<std::string, shared_ptr<BuildingObject> >					m_map_outside_spatial_structure;
	double m_recent_progress = 0;
	double m_csg_eps = 1.5e-05;
	std::map<int, std::vector<shared_ptr<StatusCallback::Message> > >	m_messages;
#ifdef ENABLE_OPENMP
	Mutex m_writelock_messages;
#endif
};

} // end namespace

#endif // IFCC_GeometryConverterH
