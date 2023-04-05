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

#pragma once

#include <map>

#include <ifcpp/geometry/AppearanceData.h>
#include <ifcpp/model/BasicTypes.h>
#include <ifcpp/model/OpenMPIncludes.h>
#include <ifcpp/model/StatusCallback.h>
#include <ifcpp/reader/ReaderUtil.h>

#include <ifcpp/IFC4X3/include/IfcColour.h>
#include <ifcpp/IFC4X3/include/IfcColourOrFactor.h>
#include <ifcpp/IFC4X3/include/IfcColourRgb.h>
#include <ifcpp/IFC4X3/include/IfcComplexProperty.h>
#include <ifcpp/IFC4X3/include/IfcCurveStyle.h>
#include <ifcpp/IFC4X3/include/IfcDraughtingPreDefinedColour.h>
#include <ifcpp/IFC4X3/include/IfcElement.h>
#include <ifcpp/IFC4X3/include/IfcExternallyDefinedHatchStyle.h>
#include <ifcpp/IFC4X3/include/IfcExternallyDefinedSurfaceStyle.h>
#include <ifcpp/IFC4X3/include/IfcFillAreaStyle.h>
#include <ifcpp/IFC4X3/include/IfcFillAreaStyleHatching.h>
#include <ifcpp/IFC4X3/include/IfcFillAreaStyleTiles.h>
#include <ifcpp/IFC4X3/include/IfcIdentifier.h>
#include <ifcpp/IFC4X3/include/IfcInteger.h>
#include <ifcpp/IFC4X3/include/IfcLabel.h>
#include <ifcpp/IFC4X3/include/IfcMaterial.h>
#include <ifcpp/IFC4X3/include/IfcMaterialDefinitionRepresentation.h>
#include <ifcpp/IFC4X3/include/IfcMaterialLayer.h>
#include <ifcpp/IFC4X3/include/IfcMaterialLayerSet.h>
#include <ifcpp/IFC4X3/include/IfcMaterialLayerSetUsage.h>
#include <ifcpp/IFC4X3/include/IfcMaterialList.h>
#include <ifcpp/IFC4X3/include/IfcNormalisedRatioMeasure.h>
#include <ifcpp/IFC4X3/include/IfcPresentationStyle.h>
#include <ifcpp/IFC4X3/include/IfcProperty.h>
#include <ifcpp/IFC4X3/include/IfcPropertySingleValue.h>
#include <ifcpp/IFC4X3/include/IfcRelAssociatesMaterial.h>
#include <ifcpp/IFC4X3/include/IfcSpecularHighlightSelect.h>
#include <ifcpp/IFC4X3/include/IfcSpecularExponent.h>
#include <ifcpp/IFC4X3/include/IfcSpecularRoughness.h>
#include <ifcpp/IFC4X3/include/IfcStyledItem.h>
#include <ifcpp/IFC4X3/include/IfcStyledRepresentation.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceStyleElementSelect.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceStyle.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceStyleShading.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceStyleRendering.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceStyleLighting.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceStyleRefraction.h>
#include <ifcpp/IFC4X3/include/IfcSurfaceStyleWithTextures.h>
#include <ifcpp/IFC4X3/include/IfcTextStyle.h>
#include <ifcpp/IFC4X3/include/IfcTextStyleTextModel.h>
#include <ifcpp/IFC4X3/include/IfcValue.h>

using namespace IFC4X3;

class StylesConverter : public StatusCallback
{
protected:
	std::map<int, shared_ptr<AppearanceData> > m_map_ifc_styles;

#ifdef _OPENMP
	Mutex m_writelock_styles_converter;
#endif

public:
	StylesConverter()
	{
	}
	virtual ~StylesConverter()
	{
	}

	void clearStylesCache()
	{
		m_map_ifc_styles.clear();
	}

	static void convertIfcSpecularHighlightSelect( shared_ptr<IfcSpecularHighlightSelect> highlight_select, shared_ptr<AppearanceData>& appearance_data )
	{
		if( dynamic_pointer_cast<IfcSpecularExponent>( highlight_select ) )
		{
			shared_ptr<IfcSpecularExponent> spec = dynamic_pointer_cast<IfcSpecularExponent>( highlight_select );
			appearance_data->m_specular_exponent = spec->m_value;
		}
		else if( dynamic_pointer_cast<IfcSpecularRoughness>( highlight_select ) )
		{
			shared_ptr<IfcSpecularRoughness> specular_roughness = dynamic_pointer_cast<IfcSpecularRoughness>( highlight_select );
			appearance_data->m_specular_roughness = specular_roughness->m_value;
		}
	}

	static void convertIfcColourRgb( shared_ptr<IfcColourRgb> color_rgb, vec4& color )
	{
		if( color_rgb->m_Red )
		{
			color.m_r = (float)color_rgb->m_Red->m_value;
		}
		if( color_rgb->m_Green )
		{
			color.m_g = (float)color_rgb->m_Green->m_value;
		}
		if( color_rgb->m_Blue )
		{
			color.m_b = (float)color_rgb->m_Blue->m_value;
		}
	}

	static void convertIfcColourOrFactor( shared_ptr<IfcColourOrFactor> color_or_factor, vec4& src_color, vec4& target_color )
	{
		// TYPE IfcColourOrFactor = SELECT ( IfcNormalisedRatioMeasure, IfcColourRgb);
		shared_ptr<IfcColourRgb> color_rgb = dynamic_pointer_cast<IfcColourRgb>( color_or_factor );
		if( color_rgb )
		{
			convertIfcColourRgb( color_rgb, target_color );
			return;
		}

		shared_ptr<IfcNormalisedRatioMeasure> ratio_measure = dynamic_pointer_cast<IfcNormalisedRatioMeasure>( color_or_factor );
		if( ratio_measure )
		{
			float factor = (float)ratio_measure->m_value;
			target_color.setColor( src_color.r()*factor, src_color.g()*factor, src_color.b()*factor, src_color.a() );
			return;
		}
	}

	static void convertIfcColour( shared_ptr<IfcColour> ifc_color_select, vec4& color )
	{
		// IfcColour = SELECT ( IfcColourSpecification, IfcPreDefinedColour );
		shared_ptr<IfcColourSpecification> color_spec = dynamic_pointer_cast<IfcColourSpecification>( ifc_color_select );
		if( color_spec )
		{
			// ENTITY IfcColourSpecification ABSTRACT SUPERTYPE OF(IfcColourRgb);
			shared_ptr<IfcColourRgb> color_rgb = dynamic_pointer_cast<IfcColourRgb>( color_spec );
			if( color_rgb )
			{
				convertIfcColourRgb( color_rgb, color );
			}
			return;
		}

		shared_ptr<IfcPreDefinedColour> predefined_color = dynamic_pointer_cast<IfcPreDefinedColour>( ifc_color_select );
		if( predefined_color )
		{
			// ENTITY IfcPreDefinedColour ABSTRACT SUPERTYPE OF(IfcDraughtingPreDefinedColour)
			shared_ptr<IfcDraughtingPreDefinedColour> draughting_predefined_color = dynamic_pointer_cast<IfcDraughtingPreDefinedColour>( predefined_color );
			if( draughting_predefined_color )
			{
				if( draughting_predefined_color->m_Name )
				{
					std::string predefined_name = draughting_predefined_color->m_Name->m_value;
					if( std_iequal( predefined_name, "black" ) )		color.setColor( 0.0, 0.0, 0.0, 1.0 );
					else if( std_iequal( predefined_name, "red" ) )		color.setColor( 1.0, 0.0, 0.0, 1.0 );
					else if( std_iequal( predefined_name, "green" ) )	color.setColor( 0.0, 1.0, 0.0, 1.0 );
					else if( std_iequal( predefined_name, "blue" ) )	color.setColor( 0.0, 0.0, 1.0, 1.0 );
					else if( std_iequal( predefined_name, "yellow" ) )	color.setColor( 1.0, 1.0, 0.0, 1.0 );
					else if( std_iequal( predefined_name, "magenta" ) )	color.setColor( 1.0, 0.0, 1.0, 1.0 );
					else if( std_iequal( predefined_name, "cyan" ) )	color.setColor( 0.0, 1.0, 1.0, 1.0 );
					else if( std_iequal( predefined_name, "white" ) )	color.setColor( 1.0, 1.0, 1.0, 1.0 );
				}
			}
			return;
		}
	}

	static void convertIfcSurfaceStyleShading(shared_ptr<IfcSurfaceStyleShading> surface_style_shading, shared_ptr<AppearanceData>& appearance_data)
	{
		if (surface_style_shading)
		{
			vec4 surface_color(0.8, 0.82, 0.84, 1.0);
			if (surface_style_shading->m_SurfaceColour)
			{
				shared_ptr<IfcColourRgb> surf_color = surface_style_shading->m_SurfaceColour;
				convertIfcColourRgb(surf_color, surface_color);
			}

			if (surface_color.r() < 0.05 && surface_color.g() < 0.05 && surface_color.b() < 0.05)
			{
				surface_color.setColor(0.11, 0.12, 0.13, surface_color.a());
			}

			vec4 ambient_color(surface_color);
			vec4 diffuse_color(surface_color);
			vec4 specular_color(surface_color);
			double shininess = 35.f;
			double alpha = surface_color.a();  // 1=opaque, 0=transparent
			bool set_transparent = false;

			shared_ptr<IfcSurfaceStyleRendering> surf_style_rendering = dynamic_pointer_cast<IfcSurfaceStyleRendering>(surface_style_shading);
			if (surf_style_rendering)
			{
				if (surf_style_rendering->m_DiffuseColour)
				{
					shared_ptr<IfcColourOrFactor> color_or_factor = surf_style_rendering->m_DiffuseColour;
					convertIfcColourOrFactor(color_or_factor, surface_color, diffuse_color);
				}

				if (surf_style_rendering->m_SpecularColour)
				{
					shared_ptr<IfcColourOrFactor> ifc_specular_color = surf_style_rendering->m_SpecularColour;
					convertIfcColourOrFactor(ifc_specular_color, surface_color, specular_color);
				}

				if (surf_style_rendering->m_Transparency)
				{
					// in IFC 1 is transparent, 0 is opaque. if not given, the value 0 (opaque) is assumed
					// in osg, 1 is opaque, 0 is transparent
					alpha = 1.f - (float)surf_style_rendering->m_Transparency->m_value;
					if (alpha < 0.1f)
					{
						alpha = 0.1f;
					}

					if (alpha > 1.f)
					{
						alpha = 1.f;
					}

					if (alpha < 0.99f)
					{
						set_transparent = true;
					}
				}

				if (surf_style_rendering->m_SpecularHighlight)
				{
					shared_ptr<IfcSpecularHighlightSelect> spec_highlight = surf_style_rendering->m_SpecularHighlight;
					convertIfcSpecularHighlightSelect(spec_highlight, appearance_data);
					shininess = appearance_data->m_specular_roughness * 128;
					if (shininess <= 1.0)
					{
						shininess = 1.0;
					}
				}
			}

			if (alpha < 0.1f)
			{
				alpha = 0.1f;
			}

			appearance_data->m_color_ambient.setColor(ambient_color.r()*0.8, ambient_color.g()*0.8, ambient_color.b()*0.8, alpha);
			appearance_data->m_color_diffuse.setColor(diffuse_color.r(), diffuse_color.g(), diffuse_color.b(), alpha);
			appearance_data->m_color_specular.setColor(specular_color.r()*0.1, specular_color.g()*0.1, specular_color.b()*0.1, alpha);

			appearance_data->m_shininess = shininess;
			appearance_data->m_set_transparent = set_transparent;
			appearance_data->m_transparency = alpha;
			appearance_data->m_complete = true;
			appearance_data->m_apply_to_geometry_type = AppearanceData::GEOM_TYPE_SURFACE;
			return;
		}
	}
	
	void convertIfcPresentationStyle( shared_ptr<IfcPresentationStyle> presentation_style, shared_ptr<AppearanceData>& appearance_data )
	{
		int style_id = presentation_style->m_tag;
#ifdef _OPENMP
		{
			ScopedLock lock(m_writelock_styles_converter);
#endif
			auto it_find_existing_style = m_map_ifc_styles.find( style_id );
			if( it_find_existing_style != m_map_ifc_styles.end() )
			{
				// use existing appearance
				appearance_data = it_find_existing_style->second;
				if( appearance_data->m_complete )
				{
					return;
				}
			}
			else
			{
				if( !appearance_data )
				{
					appearance_data = shared_ptr<AppearanceData>( new AppearanceData( style_id ) );
				}

				m_map_ifc_styles[style_id] = appearance_data;
			}
#ifdef _OPENMP
		}
#endif

		// ENTITY IfcPresentationStyle	ABSTRACT SUPERTYPE OF(ONEOF(IfcCurveStyle, IfcFillAreaStyle, IfcSurfaceStyle, IfcSymbolStyle, IfcTextStyle));
		shared_ptr<IfcCurveStyle> curve_style = dynamic_pointer_cast<IfcCurveStyle>( presentation_style );
		if( curve_style )
		{
			convertIfcCurveStyle( curve_style, appearance_data );
			return;
		}

		shared_ptr<IfcFillAreaStyle> fill_area_style = dynamic_pointer_cast<IfcFillAreaStyle>( presentation_style );
		if( fill_area_style )
		{
			//std::vector<shared_ptr<IfcFillStyleSelect> >	m_FillStyles;
			//shared_ptr<IfcBoolean>							m_ModelOrDraughting;		//optional

			appearance_data->m_apply_to_geometry_type = AppearanceData::GEOM_TYPE_SURFACE;

			for( shared_ptr<IfcFillStyleSelect>& fillStyle : fill_area_style->m_FillStyles )
			{
				// TYPE IfcFillStyleSelect = SELECT ( IfcFillAreaStyleHatching, IfcFillAreaStyleTiles, IfcExternallyDefinedHatchStyle, IfcColour);

				shared_ptr<IfcFillAreaStyleHatching> hatching = dynamic_pointer_cast<IfcFillAreaStyleHatching>(fillStyle);
				if( hatching )
				{
					// attributes:
					//shared_ptr<IfcCurveStyle>								m_HatchLineAppearance;
					//shared_ptr<IfcHatchLineDistanceSelect>					m_StartOfNextHatchLine;
					//shared_ptr<IfcCartesianPoint>							m_PointOfReferenceHatchLine;	//optional
					//shared_ptr<IfcCartesianPoint>							m_PatternStart;				//optional
					//shared_ptr<IfcPlaneAngleMeasure>						m_HatchLineAngle;

					if( hatching->m_HatchLineAppearance )
					{
						convertIfcCurveStyle(hatching->m_HatchLineAppearance, appearance_data);
					}
					continue;
				}

				shared_ptr<IfcFillAreaStyleTiles> tiles = dynamic_pointer_cast<IfcFillAreaStyleTiles>(fillStyle);
				if( tiles )
				{
#ifdef _DEBUG
					std::cout << "IfcFillAreaStyleTiles not implemented" << std::endl;
#endif
					continue;
				}

				shared_ptr<IfcExternallyDefinedHatchStyle> externalStyle = dynamic_pointer_cast<IfcExternallyDefinedHatchStyle>(fillStyle);
				if( externalStyle )
				{
#ifdef _DEBUG
					std::cout << "IfcExternallyDefinedHatchStyle not implemented" << std::endl;
#endif
					continue;
				}

				shared_ptr<IfcColour> AreaColour = dynamic_pointer_cast<IfcColour>(fillStyle);
				if( AreaColour )
				{
					vec4 color(0.2, 0.25, 0.3, 1.0);
					convertIfcColour(AreaColour, color);

					if( color.r() < 0.05 && color.g() < 0.05 && color.b() < 0.05 )
					{
						color.setColor(0.1, 0.125, 0.15, color.a());
					}

					double shininess = 35.0;
					appearance_data->m_color_ambient.setColor(color.r() * 0.8, color.g() * 0.8, color.b() * 0.8, color.a());
					appearance_data->m_color_diffuse.setColor(color.r(), color.g(), color.b(), color.a());
					appearance_data->m_color_specular.setColor(color.r() * 0.1, color.g() * 0.1, color.b() * 0.1, color.a());
					appearance_data->m_shininess = shininess;
					appearance_data->m_set_transparent = false;
					appearance_data->m_complete = true;
					continue;
				}
			}
			
			return;
		}

		shared_ptr<IfcSurfaceStyle> surface_style = dynamic_pointer_cast<IfcSurfaceStyle>( presentation_style );
		if( surface_style )
		{
			convertIfcSurfaceStyle( surface_style, appearance_data );
			return;
		}

		shared_ptr<IfcTextStyle> text_style = dynamic_pointer_cast<IfcTextStyle>( presentation_style );
		if( text_style )
		{
			appearance_data->m_text_style = text_style;
			appearance_data->m_complete = true;
			return;
		}

		return;
	}

	void convertIfcCurveStyle( shared_ptr<IfcCurveStyle> curve_style, shared_ptr<AppearanceData>& appearance_data )
	{
		if( !curve_style )
		{
			return;
		}
		int style_id = curve_style->m_tag;
#ifdef _OPENMP
		{
			ScopedLock lock(m_writelock_styles_converter);
#endif
			auto it_find_existing_style = m_map_ifc_styles.find( style_id );
			if( it_find_existing_style != m_map_ifc_styles.end() )
			{
				appearance_data = it_find_existing_style->second;
				if( appearance_data->m_complete )
				{
					return;
				}
			}
			else
			{
				if( !appearance_data )
				{
					appearance_data = shared_ptr<AppearanceData>( new AppearanceData( style_id ) );
				}

				m_map_ifc_styles[style_id] = appearance_data;
			}
#ifdef _OPENMP
		}
#endif
		appearance_data->m_apply_to_geometry_type = AppearanceData::GEOM_TYPE_CURVE;

		//CurveFont		: OPTIONAL IfcCurveFontOrScaledCurveFontSelect;
		//CurveWidth	: OPTIONAL IfcSizeSelect;
		//CurveColour	: OPTIONAL IfcColour;

		shared_ptr<IfcColour> curve_color = curve_style->m_CurveColour;
		if( curve_color )
		{
			vec4 color( 0.2, 0.25, 0.3, 1.0 );
			convertIfcColour( curve_color, color );

			if( color.r() < 0.05 && color.g() < 0.05 && color.b() < 0.05 )
			{
				color.setColor( 0.1, 0.125, 0.15, color.a() );
			}

			double shininess = 35.0;
			appearance_data->m_color_ambient.setColor( color.r()*0.8, color.g()*0.8, color.b()*0.8, color.a() );
			appearance_data->m_color_diffuse.setColor( color.r(), color.g(), color.b(), color.a() );
			appearance_data->m_color_specular.setColor( color.r()*0.1, color.g()*0.1, color.b()*0.1, color.a() );
			appearance_data->m_shininess = shininess;
			appearance_data->m_set_transparent = false;
			appearance_data->m_complete = true;
		}
	}

	void convertIfcMaterial(const shared_ptr<IfcMaterial>& mat, std::vector<shared_ptr<AppearanceData> >& vec_appearance_data)
	{
		if( !mat )
		{
			return;
		}

		// IfcMaterialDefinition -----------------------------------------------------------
		// inverse attributes:
		//  std::vector<weak_ptr<IfcRelAssociatesMaterial> >			m_AssociatedTo_inverse;
		//  std::vector<weak_ptr<IfcExternalReferenceRelationship> >	m_HasExternalReferences_inverse;
		//  std::vector<weak_ptr<IfcMaterialProperties> >				m_HasProperties_inverse;

		// IfcMaterial -----------------------------------------------------------
		// attributes:
		shared_ptr<IfcLabel>& Name = mat->m_Name;
		//shared_ptr<IfcText>											m_Description;				//optional
		//shared_ptr<IfcLabel>										m_Category;					//optional
		// inverse attributes:
		std::vector<weak_ptr<IfcMaterialDefinitionRepresentation> >	m_HasRepresentation_inverse;
		std::vector<weak_ptr<IfcMaterialRelationship> >				m_IsRelatedWith_inverse;
		std::vector<weak_ptr<IfcMaterialRelationship> >				m_RelatesTo_inverse;


		for( size_t kk = 0; kk < mat->m_IsRelatedWith_inverse.size(); ++kk )
		{
			const weak_ptr<IfcMaterialRelationship>& mat_is_related_weak = mat->m_IsRelatedWith_inverse[kk];

			if( mat_is_related_weak.expired() )
			{
				continue;
			}

			shared_ptr<IfcMaterialRelationship> mat_is_related(mat_is_related_weak);
			if( !mat_is_related )
			{
				continue;
			}

		}


		//#132199 = IFCCOLOURRGB($, 0.933333333333333, 0.929411764705882, 0.756862745098039);
		//#132200 = IFCSURFACESTYLERENDERING(#132199, 0., $, $, $, $, IFCNORMALISEDRATIOMEASURE(0.5), IFCSPECULAREXPONENT(64.), .NOTDEFINED.);
		//#132201 = IFCSURFACESTYLE('Wall paint', .BOTH., (#132200));
		//#132203 = IFCPRESENTATIONSTYLEASSIGNMENT((#132201));
		//#132205 = IFCSTYLEDITEM(#132198, (#132203), $);

		//#132228 = IFCPRESENTATIONSTYLEASSIGNMENT((#132201));
		//#132230 = IFCSTYLEDITEM($, (#132228), $);
		//#132232 = IFCSTYLEDREPRESENTATION(#96, 'Style', 'Material', (#132230));
		//#132234 = IFCMATERIALDEFINITIONREPRESENTATION($, $, (#132232), #132227);

		//#132227 = IFCMATERIAL('Wall paint');
		//#132351 = IFCWALLSTANDARDCASE('3RLTfSMBXE_BWgA7WbvgI5', #41, 'Mur de base:250 mm_Exterior Wall:188968', $, 'Mur de base:250 mm_Exterior Wall:190858', #132331, #132349, '188968');
		//#132366 = IFCMATERIALLAYER(#132227, 0.25, $);
		//#132367 = IFCMATERIALLAYERSET((#132366), 'Mur de base:250 mm_Exterior Wall');
		//#132370 = IFCMATERIALLAYERSETUSAGE(#132367, .AXIS2., .POSITIVE., -0.125);

		//#805426 = IFCRELASSOCIATESMATERIAL('3UrmiAemXCIO2heyEAUvHP', #41, $, $, (#132351), #132370);


		for( size_t kk = 0; kk < mat->m_HasRepresentation_inverse.size(); ++kk )
		{
			const weak_ptr<IfcMaterialDefinitionRepresentation>& mat_def_representation_weak = mat->m_HasRepresentation_inverse[kk];

			if( mat_def_representation_weak.expired() )
			{
				continue;
			}

			shared_ptr<IfcMaterialDefinitionRepresentation> mat_def_representation(mat_def_representation_weak);
			if( !mat_def_representation )
			{
				continue;
			}
			std::vector<shared_ptr<IfcRepresentation> >& mat_representations = mat_def_representation->m_Representations;

			for( const shared_ptr<IfcRepresentation>& mat_rep : mat_representations )
			{
				if( !mat_rep )
				{
					continue;
				}
				shared_ptr<IfcStyledRepresentation> mat_style = dynamic_pointer_cast<IfcStyledRepresentation>(mat_rep);
				if( !mat_style )
				{
					continue;
				}
				for( const shared_ptr<IfcRepresentationItem>& mat_rep_item : mat_style->m_Items )
				{
					if( !mat_rep_item )
					{
						continue;
					}

					shared_ptr<IfcStyledItem> styled_item = dynamic_pointer_cast<IfcStyledItem>(mat_rep_item);
					if( styled_item )
					{
						//std::vector<shared_ptr<IfcPresentationStyle> >			m_Styles;
						for( const shared_ptr<IfcPresentationStyle>& presentationStyle : styled_item->m_Styles )
						{
							if( !presentationStyle )
							{
								continue;
							}

							shared_ptr<AppearanceData> appearance_data;
							convertIfcPresentationStyle( presentationStyle, appearance_data );
							if( appearance_data )
							{
								vec_appearance_data.push_back( appearance_data );
							}


							// ENTITY IfcPresentationStyle   ABSTRACT SUPERTYPE OF (ONEOF (IfcCurveStyle ,IfcFillAreaStyle ,IfcSurfaceStyle ,IfcTextStyle));
							//shared_ptr<AppearanceData> appearance_data;

							//shared_ptr<IfcCurveStyle> curveStyle = dynamic_pointer_cast<IfcCurveStyle>(style);
							//if( curveStyle )
							//{
							//	convertIfcCurveStyle( curve_style, appearance_data );
							//	if( appearance_data )
							//	{
							//		vec_appearance_data.push_back( appearance_data );
							//	}
							//	continue;
							//}

							//		shared_ptr<IfcSurfaceStyle> surface_style = dynamic_pointer_cast<IfcSurfaceStyle>(style_select);
							//		if( !surface_style )
							//		{
							//			continue;
							//		}
							//		for( const shared_ptr<IfcSurfaceStyleElementSelect>& surface_style_select : surface_style->m_Styles )
							//		{
							//			if( !surface_style_select )
							//			{
							//				continue;
							//			}
							//			shared_ptr<IfcSurfaceStyleShading> surface_style_shading = dynamic_pointer_cast<IfcSurfaceStyleShading>(surface_style_select);
							//			if( surface_style_shading )
							//			{
							//				const int style_id = surface_style->m_tag;
							//				shared_ptr<AppearanceData> appearance_data(new AppearanceData(style_id));
							//				StylesConverter::convertIfcSurfaceStyleShading(surface_style_shading, appearance_data);
							//				vec_appearance_data.push_back(appearance_data);
							//				continue;
							//			}
							//		}
							//	}
							//}
						}
					}
				}
			}
		}
	}

	void convertElementStyle(const shared_ptr<IfcElement>& ifc_element, std::vector<shared_ptr<AppearanceData> >& vec_appearance_data)
	{
		// handle assigned materials
		std::vector<weak_ptr<IfcRelAssociates> >& vec_associates = ifc_element->m_HasAssociations_inverse;

		for (size_t ii = 0; ii < vec_associates.size(); ++ii)
		{
			shared_ptr<IfcRelAssociates> rel_associates(vec_associates[ii]);
			shared_ptr<IfcRelAssociatesMaterial> associated_material = dynamic_pointer_cast<IfcRelAssociatesMaterial>(rel_associates);
			if (!associated_material)
			{
				continue;
			}

			// TYPE IfcMaterialSelect = SELECT	(IfcMaterialDefinition	,IfcMaterialList	,IfcMaterialUsageDefinition);
			shared_ptr<IfcMaterialSelect> relatingMaterial = associated_material->m_RelatingMaterial;

			if (!relatingMaterial )
			{
				continue;
			}

			shared_ptr<IfcMaterial> material = dynamic_pointer_cast<IfcMaterial>(relatingMaterial);
			if( material )
			{
				convertIfcMaterial(material, vec_appearance_data);
			}

			shared_ptr<IfcMaterialList> materialList = dynamic_pointer_cast<IfcMaterialList>(relatingMaterial);
			if( materialList )
			{

				for( size_t jj = 0; jj < materialList->m_Materials.size(); ++jj )
				{
					const shared_ptr<IfcMaterial>& mat = materialList->m_Materials[jj];
					convertIfcMaterial(mat, vec_appearance_data);
				}

				continue;
			}

			shared_ptr<IfcMaterialLayerSetUsage> material_layer_set_usage = dynamic_pointer_cast<IfcMaterialLayerSetUsage>(relatingMaterial);
			if( material_layer_set_usage )
			{


				if( !material_layer_set_usage->m_ForLayerSet )
				{
					continue;
				}

				for( size_t jj = 0; jj < material_layer_set_usage->m_ForLayerSet->m_MaterialLayers.size(); ++jj )
				{
					const shared_ptr<IfcMaterialLayer>& material_layer = material_layer_set_usage->m_ForLayerSet->m_MaterialLayers[jj];
					if( !material_layer )
					{
						continue;
					}

					const shared_ptr<IfcMaterial>& mat = material_layer->m_Material;					//optional
					convertIfcMaterial(mat, vec_appearance_data);

				}
			}
		}
	}

	void convertIfcSurfaceStyle( shared_ptr<IfcSurfaceStyle> surface_style, shared_ptr<AppearanceData>& appearance_data )
	{
		if( !surface_style )
		{
			return;
		}
		const int style_id = surface_style->m_tag;

#ifdef _OPENMP
		{
		ScopedLock lock(m_writelock_styles_converter);
#endif
		auto it_find_existing_style = m_map_ifc_styles.find( style_id );
		if( it_find_existing_style != m_map_ifc_styles.end() )
		{
			// todo: check if appearance compare is faster here
			appearance_data = it_find_existing_style->second;
			if( appearance_data->m_complete )
			{
				return;
			}
		}
		else
		{
			if( !appearance_data )
			{
				appearance_data = shared_ptr<AppearanceData>( new AppearanceData( style_id ) );
			}

			m_map_ifc_styles[style_id] = appearance_data;
		}

#ifdef _OPENMP
		}
#endif

		appearance_data->m_apply_to_geometry_type = AppearanceData::GEOM_TYPE_SURFACE;

		std::vector<shared_ptr<IfcSurfaceStyleElementSelect> >& vec_styles = surface_style->m_Styles;
		if( vec_styles.size() == 0 )
		{
			return;
		}

		for( size_t ii_styles = 0; ii_styles < vec_styles.size(); ++ii_styles )
		{
			shared_ptr<IfcSurfaceStyleElementSelect> surf_style_element_select = vec_styles[ii_styles];
			if( !surf_style_element_select )
			{
				continue;
			}
			// TYPE IfcSurfaceStyleElementSelect = SELECT	(IfcExternallyDefinedSurfaceStyle	,IfcSurfaceStyleLighting	,IfcSurfaceStyleRefraction	,IfcSurfaceStyleShading	,IfcSurfaceStyleWithTextures);
			shared_ptr<IfcSurfaceStyleShading> surface_style_shading = dynamic_pointer_cast<IfcSurfaceStyleShading>( surf_style_element_select );
			if( surface_style_shading )
			{
				convertIfcSurfaceStyleShading(surface_style_shading, appearance_data);
				continue;
			}

			shared_ptr<IfcExternallyDefinedSurfaceStyle> ext_surf_style = dynamic_pointer_cast<IfcExternallyDefinedSurfaceStyle>( surf_style_element_select );
			if( ext_surf_style )
			{
#ifdef _DEBUG
				std::cout << "IfcExternallyDefinedSurfaceStyle not implemented" << std::endl;
#endif
				continue;
			}

			shared_ptr<IfcSurfaceStyleLighting> style_lighting = dynamic_pointer_cast<IfcSurfaceStyleLighting>( surf_style_element_select );
			if( style_lighting )
			{
#ifdef _DEBUG
				std::cout << "IfcSurfaceStyleLighting not implemented" << std::endl;
#endif
				continue;
			}

			shared_ptr<IfcSurfaceStyleRefraction> style_refraction = dynamic_pointer_cast<IfcSurfaceStyleRefraction>( surf_style_element_select );
			if( style_refraction )
			{
#ifdef _DEBUG
				std::cout << "IfcSurfaceStyleRefraction not implemented" << std::endl;
#endif
				continue;
			}

			shared_ptr<IfcSurfaceStyleWithTextures> style_texture = dynamic_pointer_cast<IfcSurfaceStyleWithTextures>( surf_style_element_select );
			if( style_texture )
			{
#ifdef _DEBUG
				std::cout << "IfcSurfaceStyleWithTextures not implemented" << std::endl;
#endif
				continue;
			}
		}
	}

	void convertIfcStyledItem( weak_ptr<IfcStyledItem> styled_item_weak, std::vector<shared_ptr<AppearanceData> >& vec_appearance_data )
	{
		if( styled_item_weak.expired() )
		{
			return;
		}
		shared_ptr<IfcStyledItem> styled_item( styled_item_weak );
		const int style_id = styled_item->m_tag;

#ifdef _OPENMP
		{
		ScopedLock lock(m_writelock_styles_converter);
#endif
		auto it_find_existing_style = m_map_ifc_styles.find( style_id );
		if( it_find_existing_style != m_map_ifc_styles.end() )
		{
			vec_appearance_data.push_back( it_find_existing_style->second );
			return;
		}
	
#ifdef _OPENMP
		}
#endif

		std::vector<shared_ptr<IfcPresentationStyle> >& vec_style_assigns = styled_item->m_Styles;
		for( size_t i_style_assign = 0; i_style_assign < vec_style_assigns.size(); ++i_style_assign )
		{
			shared_ptr<IfcPresentationStyle> presentationStyle = vec_style_assigns[i_style_assign];
			if( !presentationStyle )
			{
				continue;
			}

			shared_ptr<AppearanceData> appearance_data;
			convertIfcPresentationStyle( presentationStyle, appearance_data );
			if( appearance_data )
			{
				vec_appearance_data.push_back( appearance_data );
			}
			continue;
		}
	}

	void convertIfcComplexPropertyColor( shared_ptr<IfcComplexProperty> complex_property, vec4& vec_color )
	{
		std::vector<shared_ptr<IfcProperty> >& vec_HasProperties = complex_property->m_HasProperties;
		if( !complex_property->m_UsageName ) return;
		if( vec_HasProperties.size() < 3 ) return;
		std::string usage_name = complex_property->m_UsageName->m_value;
		if( !std_iequal( usage_name.c_str(), "Color" ) ) return;

		if( complex_property->m_HasProperties.size() > 2 )
		{
			shared_ptr<IfcPropertySingleValue> prop1 = dynamic_pointer_cast<IfcPropertySingleValue>( complex_property->m_HasProperties[0] );
			shared_ptr<IfcPropertySingleValue> prop2 = dynamic_pointer_cast<IfcPropertySingleValue>( complex_property->m_HasProperties[1] );
			shared_ptr<IfcPropertySingleValue> prop3 = dynamic_pointer_cast<IfcPropertySingleValue>( complex_property->m_HasProperties[2] );

			if( prop1 && prop2 && prop3 )
			{
				shared_ptr<IfcValue>	v1_select = prop1->m_NominalValue;
				shared_ptr<IfcValue>	v2_select = prop2->m_NominalValue;
				shared_ptr<IfcValue>	v3_select = prop3->m_NominalValue;
				if( v1_select && v2_select && v3_select )
				{
					shared_ptr<IfcInteger> v1_int = dynamic_pointer_cast<IfcInteger>( v1_select );
					shared_ptr<IfcInteger> v2_int = dynamic_pointer_cast<IfcInteger>( v2_select );
					shared_ptr<IfcInteger> v3_int = dynamic_pointer_cast<IfcInteger>( v3_select );

					if( v1_int && v2_int && v3_int )
					{
						double r = v1_int->m_value / 255.0;
						double g = v2_int->m_value / 255.0;
						double b = v3_int->m_value / 255.0;

						if( r < 0.05 && g < 0.05 && b < 0.05 )
						{
							r = 0.1;
							g = 0.12;
							b = 0.15;
						}
						vec_color.m_r = r;
						vec_color.m_g = g;
						vec_color.m_b = b;
						vec_color.m_a = 1.0;

#ifdef _OPENMP
						//ScopedLock lock( m_writelock_styles_converter );
#endif
						//appearance_data->color_ambient.setColor( r, g, b, 1.f );
						//appearance_data->color_diffuse.setColor( r, g, b, 1.f );
						//appearance_data->color_specular.setColor( r, g, b, 1.f );
						//appearance_data->shininess = 35.f;

						//m_map_ifc_styles[complex_property_id] = appearance_data;

						return;
					}
				}
			}
		}
	}
};
