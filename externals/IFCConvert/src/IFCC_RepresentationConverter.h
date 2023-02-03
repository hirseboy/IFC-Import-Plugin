#ifndef IFCC_RepresentationConverterH
#define IFCC_RepresentationConverterH

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

#include <ifcpp/geometry/StylesConverter.h>

#include <ifcpp/IFC4/include/IfcPropertySet.h>

#include "IFCC_TessellatedItemConverter.h"
#include "IFCC_SolidModelConverter.h"
#include "IFCC_FaceConverter.h"
#include "IFCC_SolidModelConverter.h"
#include "IFCC_CurveConverter.h"
#include "IFCC_ProfileCache.h"
#include "IFCC_Types.h"

namespace IFCC {

/*! Class for converting all types of geometrical representation into shape data.*/
class RepresentationConverter : public StatusCallback
{
public:
	RepresentationConverter( shared_ptr<GeometrySettings> geom_settings, shared_ptr<UnitConverter> unit_converter );

	virtual ~RepresentationConverter();

	void clearCache();

	shared_ptr<GeometrySettings>&		getGeomSettings();
	shared_ptr<UnitConverter>&			getUnitConverter();
	shared_ptr<StylesConverter>&		getStylesConverter();
	shared_ptr<PointConverter>&			getPointConverter();
	shared_ptr<SplineConverter>&		getSplineConverter() ;
	shared_ptr<Sweeper>&				getSweeper();
	shared_ptr<PlacementConverter>&		getPlacementConverter();
	shared_ptr<CurveConverter>&			getCurveConverter();
	shared_ptr<ProfileCache>&			getProfileCache();
	shared_ptr<FaceConverter>&			getFaceConverter();
	shared_ptr<SolidModelConverter>&	getSolidConverter();

	void setUnitConverter( shared_ptr<UnitConverter>& unit_converter );

	void convertRepresentationStyle( const shared_ptr<IfcRepresentationItem>& representation_item, std::vector<shared_ptr<AppearanceData> >& vec_appearance_data );

	bool convertIfcRepresentation( const shared_ptr<IfcRepresentation>& ifc_representation, shared_ptr<RepresentationData>& representation_data, std::vector<ConvertError>& errors );

	bool convertIfcGeometricRepresentationItem( const shared_ptr<IfcGeometricRepresentationItem>& geom_item, shared_ptr<ItemShapeData> item_data, std::vector<ConvertError>& errors );

	void convertTopologicalRepresentationItem( const shared_ptr<IfcTopologicalRepresentationItem>& topological_item, shared_ptr<ItemShapeData> topo_item_data );

	bool subtractOpenings( const shared_ptr<IfcElement>& ifc_element, shared_ptr<ProductShapeData>& product_shape, std::vector<ConvertError>& errors );

protected:
	shared_ptr<GeometrySettings>		m_geom_settings;
	shared_ptr<UnitConverter>			m_unit_converter;
	shared_ptr<StylesConverter>			m_styles_converter;
	shared_ptr<PointConverter>			m_point_converter;
	shared_ptr<SplineConverter>			m_spline_converter;
	shared_ptr<Sweeper>					m_sweeper;
	shared_ptr<PlacementConverter>		m_placement_converter;
	shared_ptr<CurveConverter>			m_curve_converter;
	shared_ptr<ProfileCache>			m_profile_cache;
	shared_ptr<FaceConverter>			m_face_converter;
	shared_ptr<SolidModelConverter>		m_solid_converter;
	shared_ptr<TessellatedItemConverter> m_tessel_converter;

};

} // end namespace

#endif // IFCC_RepressentationConverterH
