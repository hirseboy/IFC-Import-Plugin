#include "IFCC_Property.h"

#include <ifcpp/IFC4X3/include/IfcPropertySetDefinitionSelect.h>
#include <ifcpp/IFC4X3/include/IfcPropertySetDefinition.h>
#include <ifcpp/IFC4X3/include/IfcPropertySet.h>
#include <ifcpp/IFC4X3/include/IfcPropertySingleValue.h>
#include <ifcpp/IFC4X3/include/IfcPropertyBoundedValue.h>
#include <ifcpp/IFC4X3/include/IfcPropertyEnumeratedValue.h>
#include <ifcpp/IFC4X3/include/IfcPropertyListValue.h>
#include <ifcpp/IFC4X3/include/IfcPropertyReferenceValue.h>
#include <ifcpp/IFC4X3/include/IfcPropertyTableValue.h>

#include <ifcpp/IFC4X3/include/IfcMaterialProperties.h>

#include <ifcpp/IFC4X3/include/IfcDerivedMeasureValue.h>
#include <ifcpp/IFC4X3/include/IfcMeasureValue.h>
#include <ifcpp/IFC4X3/include/IfcSimpleValue.h>
#include <ifcpp/IFC4X3/include/IfcBinary.h>
#include <ifcpp/IFC4X3/include/IfcBoolean.h>
#include <ifcpp/IFC4X3/include/IfcDate.h>
#include <ifcpp/IFC4X3/include/IfcDateTime.h>
#include <ifcpp/IFC4X3/include/IfcDuration.h>
#include <ifcpp/IFC4X3/include/IfcReal.h>
#include <ifcpp/IFC4X3/include/IfcPositiveInteger.h>
#include <ifcpp/IFC4X3/include/IfcTime.h>
#include <ifcpp/IFC4X3/include/IfcTimeStamp.h>
#include <ifcpp/IFC4X3/include/IfcLogical.h>

#include <ifcpp/IFC4X3/include/IfcHeatFluxDensityMeasure.h>
#include <ifcpp/IFC4X3/include/IfcIsothermalMoistureCapacityMeasure.h>
#include <ifcpp/IFC4X3/include/IfcThermalConductivityMeasure.h>
#include <ifcpp/IFC4X3/include/IfcThermalResistanceMeasure.h>
#include <ifcpp/IFC4X3/include/IfcThermalTransmittanceMeasure.h>
#include <ifcpp/IFC4X3/include/IfcVaporPermeabilityMeasure.h>

#include <Carve/src/include/carve/carve.hpp>
#include <ifcpp/geometry/MeshUtils.h>

#include "IFCC_Helper.h"

namespace IFCC {

Property::Property() :
	m_valueType(VT_NotDefined),
	m_boolValue(false),
	m_intValue(-1),
	m_doubleValue(-1e30)
{}

bool Property::relevantProperty(const std::string& propSetName, const std::string& propName) {
	if(propSetName == "Pset_WindowCommon") {
		if(propName == "IsExternal")
			return true;
		if(propName == "Infiltration")
			return true;
		if(propName == "ThermalTransmittance")
			return true;
		if(propName == "GlazingAreaFraction")
			return true;
	}
	else if(propSetName == "Pset_WallCommon") {
		if(propName == "IsExternal")
			return true;
		if(propName == "ThermalTransmittance")
			return true;
	}
	else if(propSetName == "Pset_RoofCommon") {
		if(propName == "IsExternal")
			return true;
		if(propName == "ProjectedArea")
			return true;
		if(propName == "TotalArea")
			return true;
	}
	else if(propSetName == "Pset_SlabCommon") {
		if(propName == "IsExternal")
			return true;
		if(propName == "PitchAngle")
			return true;
	}
	else if(propSetName == "Pset_DoorCommon") {
		if(propName == "IsExternal")
			return true;
		if(propName == "ThermalTransmittance")
			return true;
		if(propName == "Infiltration")
			return true;
		if(propName == "GlazingAreaFraction")
			return true;
	}
	else if(propSetName == "Pset_DoorWindowGlazingCommon") {
		if(propName == "GlassLayers")
			return true;
		if(propName == "GlassThickness1")
			return true;
		if(propName == "GlassThickness2")
			return true;
		if(propName == "GlassThickness3")
			return true;
		if(propName == "FillGas")
			return true;
		if(propName == "Translucency")
			return true;
		if(propName == "Reflectivity")
			return true;
		if(propName == "SolarHeatGainTransmittance")
			return true;
	}
	else if(propSetName == "Pset_DoorWindowShadingCommon") {
		if(propName == "ExternalShadingCoefficient")
			return true;
		if(propName == "InternalShadingCoefficient")
			return true;
		if(propName == "InsetShadingCoefficient")
			return true;
	}
	else if(propSetName == "Pset_MaterialCommon") {
		if(propName == "Porosity")
			return true;
		if(propName == "MassDensity")
			return true;
	}
	else if(propSetName == "Pset_MaterialHygroscopic") {
		if(propName == "UpperVaporResistanceFactor")
			return true;
		if(propName == "LowerVaporResistanceFactor")
			return true;
		if(propName == "IsothermalMoistureCapacity")
			return true;
		if(propName == "VaporPermeability")
			return true;
		if(propName == "MoistureDiffusivity")
			return true;
	}
	else if(propSetName == "Pset_MaterialOptical") {
		if(propName == "VisibleTransmittance")
			return true;
		if(propName == "SolarTransmittance")
			return true;
		if(propName == "ThermalIrTransmittance")
			return true;
		if(propName == "ThermalIrEmissivityBack")
			return true;
		if(propName == "ThermalIrEmissivityFront")
			return true;
		if(propName == "VisibleReflectanceBack")
			return true;
		if(propName == "VisibleReflectanceFront")
			return true;
		if(propName == "SolarReflectanceBack")
			return true;
		if(propName == "SolarReflectanceFront")
			return true;
	}
	else if(propSetName == "Pset_MaterialThermal") {
		if(propName == "SpecificHeatCapacity")
			return true;
		if(propName == "ThermalConductivity")
			return true;
	}

	return false;
}

bool isSimpleProperty(shared_ptr<IFC4X3::IfcProperty> property) {
	shared_ptr<IFC4X3::IfcSimpleProperty> simpleValue = dynamic_pointer_cast<IFC4X3::IfcSimpleProperty>(property);
	if(!simpleValue)
		return true;
}

template<class T>
bool setDoubleProp(shared_ptr<IFC4X3::IfcPropertySingleValue> singleValue, Property& prop) {
	shared_ptr<T> value1 = dynamic_pointer_cast<T>(singleValue->m_NominalValue);
	if(value1) {
//		prop.m_description = value1->m_tag;
		prop.m_valueType = Property::VT_Double;
		prop.m_doubleValue = value1->m_value;
		return true;
	}
	return false;
}

bool getProperty(shared_ptr<IFC4X3::IfcProperty> property, const std::string& pset_name, Property& prop) {

	if(!isSimpleProperty(property))
		return false;

	shared_ptr<IFC4X3::IfcPropertyBoundedValue> boundedValue = dynamic_pointer_cast<IFC4X3::IfcPropertyBoundedValue>(property);
	shared_ptr<IFC4X3::IfcPropertyEnumeratedValue> enumeratedValue = dynamic_pointer_cast<IFC4X3::IfcPropertyEnumeratedValue>(property);
	shared_ptr<IFC4X3::IfcPropertyListValue> listValue = dynamic_pointer_cast<IFC4X3::IfcPropertyListValue>(property);
	shared_ptr<IFC4X3::IfcPropertyReferenceValue> referenceValue = dynamic_pointer_cast<IFC4X3::IfcPropertyReferenceValue>(property);
	shared_ptr<IFC4X3::IfcPropertySingleValue> singleValue = dynamic_pointer_cast<IFC4X3::IfcPropertySingleValue>(property);
	if(singleValue) {
		if(!singleValue->m_NominalValue)
			return false;

		shared_ptr<IFC4X3::IfcDerivedMeasureValue> dmv = dynamic_pointer_cast<IFC4X3::IfcDerivedMeasureValue>(singleValue->m_NominalValue);
		if(dmv) {
			if(setDoubleProp<IFC4X3::IfcHeatFluxDensityMeasure>(singleValue, prop))
				return true;
			if(setDoubleProp<IFC4X3::IfcIsothermalMoistureCapacityMeasure>(singleValue, prop))
				return true;
			if(setDoubleProp<IFC4X3::IfcThermalConductivityMeasure>(singleValue, prop))
				return true;
			if(setDoubleProp<IFC4X3::IfcThermalResistanceMeasure>(singleValue, prop))
				return true;
			if(setDoubleProp<IFC4X3::IfcThermalTransmittanceMeasure>(singleValue, prop))
				return true;
			if(setDoubleProp<IFC4X3::IfcVaporPermeabilityMeasure>(singleValue, prop))
				return true;


//	all other values
//					IfcAbsorbedDoseMeasure
//					IfcAccelerationMeasure
//					IfcAngularVelocityMeasure
//					IfcAreaDensityMeasure
//					IfcCompoundPlaneAngleMeasure
//					IfcCurvatureMeasure
//					IfcDoseEquivalentMeasure
//					IfcDynamicViscosityMeasure
//					IfcElectricCapacitanceMeasure
//					IfcElectricChargeMeasure
//					IfcElectricConductanceMeasure
//					IfcElectricResistanceMeasure
//					IfcElectricVoltageMeasure
//					IfcEnergyMeasure
//					IfcForceMeasure
//					IfcFrequencyMeasur
//					IfcHeatingValueMeasure
//					IfcIlluminanceMeasure
//					IfcInductanceMeasure
//					IfcIntegerCountRateMeasure
//					IfcIonConcentrationMeasure
//					IfcKinematicViscosityMeasure
//					IfcLinearForceMeasure
//					IfcLinearMomentMeasure
//					IfcLinearStiffnessMeasure
//					IfcLinearVelocityMeasure
//					IfcLuminousFluxMeasure
//					IfcLuminousIntensityDistributionMeasure
//					IfcMagneticFluxDensityMeasure
//					IfcMagneticFluxMeasure
//					IfcMassDensityMeasure
//					IfcMassFlowRateMeasure
//					IfcMassPerLengthMeasure
//					IfcModulusOfElasticityMeasure
//					IfcModulusOfLinearSubgradeReactionMeasure
//					IfcModulusOfRotationalSubgradeReactionMeasure
//					IfcModulusOfSubgradeReactionMeasure
//					IfcMoistureDiffusivityMeasure
//					IfcMolecularWeightMeasure
//					IfcMomentOfInertiaMeasure
//					IfcMonetaryMeasure
//					IfcPHMeasure
//					IfcPlanarForceMeasure
//					IfcPowerMeasure
//					IfcPressureMeasure
//					IfcRadioActivityMeasure
//					IfcRotationalFrequencyMeasure
//					IfcRotationalMassMeasure
//					IfcRotationalStiffnessMeasure
//					IfcSectionModulusMeasure
//					IfcSectionalAreaIntegralMeasure
//					IfcShearModulusMeasure
//					IfcSoundPowerLevelMeasure
//					IfcSoundPowerMeasure
//					IfcSoundPressureLevelMeasure
//					IfcSoundPressureMeasure
//					IfcSpecificHeatCapacityMeasure
//					IfcTemperatureGradientMeasure
//					IfcTemperatureRateOfChangeMeasure
//					IfcThermalAdmittanceMeasure
//					IfcThermalExpansionCoefficientMeasure
//					IfcTorqueMeasure
//					IfcVolumetricFlowRateMeasure
//					IfcWarpingConstantMeasure
//					IfcWarpingMomentMeasure
		}
		shared_ptr<IFC4X3::IfcMeasureValue> mv = dynamic_pointer_cast<IFC4X3::IfcMeasureValue>(singleValue->m_NominalValue);
		if(mv) {
//			IfcAmountOfSubstanceMeasure
//			IfcAreaMeasure
//			IfcComplexNumber
//			IfcContextDependentMeasure
//			IfcCountMeasure
//			IfcDescriptiveMeasure
//			IfcElectricCurrentMeasure
//			IfcLengthMeasure
//			IfcLuminousIntensityMeasure
//			IfcMassMeasure
//			IfcNonNegativeLengthMeasure
//			IfcNormalisedRatioMeasure
//			IfcNumericMeasure
//			IfcParameterValue
//			IfcPlaneAngleMeasure
//			IfcPositiveLengthMeasure
//			IfcPositivePlaneAngleMeasure
//			IfcPositiveRatioMeasure
//			IfcRatioMeasure
//			IfcSolidAngleMeasure
//			IfcThermodynamicTemperatureMeasure
//			IfcTimeMeasure
//			IfcVolumeMeasure
		}
		shared_ptr<IFC4X3::IfcSimpleValue> sv = dynamic_pointer_cast<IFC4X3::IfcSimpleValue>(singleValue->m_NominalValue);
		if(sv) {
			if(sv) {
				if(dynamic_pointer_cast<IFC4X3::IfcBinary>(sv)) {
					prop.m_valueType = Property::VT_String;
					prop.m_stringValue = dynamic_pointer_cast<IFC4X3::IfcBinary>(sv)->m_value;
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcBoolean>(sv)) {
					prop.m_valueType = Property::VT_Boolean;
					prop.m_boolValue = dynamic_pointer_cast<IFC4X3::IfcBoolean>(sv)->m_value;
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcDate>(sv)) {

				}
				else if(dynamic_pointer_cast<IFC4X3::IfcDateTime>(sv)) {

				}
				else if(dynamic_pointer_cast<IFC4X3::IfcDuration>(sv)) {

				}
				else if(dynamic_pointer_cast<IFC4X3::IfcIdentifier>(sv)) {
					prop.m_valueType = Property::VT_String;
					prop.m_stringValue = dynamic_pointer_cast<IFC4X3::IfcIdentifier>(sv)->m_value;
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcInteger>(sv)) {
					prop.m_valueType = Property::VT_INT;
					prop.m_intValue = dynamic_pointer_cast<IFC4X3::IfcInteger>(sv)->m_value;
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcLabel>(sv)) {
					prop.m_valueType = Property::VT_String;
					prop.m_stringValue = label2s(dynamic_pointer_cast<IFC4X3::IfcLabel>(sv));
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcLogical>(sv)) {
					prop.m_valueType = Property::VT_Boolean;
					prop.m_boolValue = dynamic_pointer_cast<IFC4X3::IfcLogical>(sv)->m_value == LOGICAL_TRUE;
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcPositiveInteger>(sv)) {
					prop.m_valueType = Property::VT_INT;
					prop.m_intValue = dynamic_pointer_cast<IFC4X3::IfcPositiveInteger>(sv)->m_value;
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcReal>(sv)) {
					prop.m_valueType = Property::VT_Double;
					prop.m_doubleValue = dynamic_pointer_cast<IFC4X3::IfcReal>(sv)->m_value;
					return true;
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcText>(sv)) {
					prop.m_valueType = Property::VT_String;
					return true;
					prop.m_stringValue = text2s(dynamic_pointer_cast<IFC4X3::IfcText>(sv));
				}
				else if(dynamic_pointer_cast<IFC4X3::IfcTime>(sv)) {

				}
				else if(dynamic_pointer_cast<IFC4X3::IfcTimeStamp>(sv)) {

				}
			}
		}
	}
	shared_ptr<IFC4X3::IfcPropertyTableValue> tableValue = dynamic_pointer_cast<IFC4X3::IfcPropertyTableValue>(property);
	return false;
}


bool getDoubleProperty(const std::map<std::string,std::map<std::string,Property>>& pmap,
								 const std::string& setName, const std::string& name, double& value ) {
	const auto& set_map = pmap.find(setName);
	if(set_map == pmap.end())
		return false;

	const auto& name_map = set_map->second.find(name);
	if(name_map == set_map->second.end())
		return false;

	const Property& prop = name_map->second;
	if(prop.m_valueType == Property::VT_Double) {
		value = prop.m_doubleValue;
		return true;
	}

	return false;
}

void getMaterialProperties(const shared_ptr<IFC4X3::IfcMaterial>& mat, std::map<std::string,std::map<std::string,Property>>& propItem) {
	for(const auto& relproperties : mat->m_HasProperties_inverse) {
		shared_ptr<IFC4X3::IfcMaterialProperties> mat_properties(relproperties);
		if(mat_properties) {
			std::string pset_name = name2s(mat_properties->m_Name);
			for(const auto& property : mat_properties->m_Properties) {
				std::string name = name2s(property->m_Name);
				bool usesThisProperty = Property::relevantProperty(pset_name,name);
				if(usesThisProperty) {
					Property prop;
					prop.m_name = name;
					getProperty(property,pset_name, prop);
					std::map<std::string, Property> inner;
					inner.insert(std::make_pair(name, prop));
					propItem.insert(std::make_pair(pset_name, inner));
				}
			}
		}
	}
}


} // namespace IFCC
