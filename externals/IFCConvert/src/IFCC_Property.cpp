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
#include <ifcpp/IFC4X3/include/IfcRelDefinesByProperties.h>
#include <ifcpp/IFC4X3/include/IfcPropertySetDefinitionSet.h>
#include <ifcpp/IFC4X3/include/IfcPreDefinedPropertySet.h>

#include <ifcpp/IFC4X3/include/IfcQuantitySet.h>
#include <ifcpp/IFC4X3/include/IfcElementQuantity.h>
#include <ifcpp/IFC4X3/include/IfcQuantityArea.h>
#include <ifcpp/IFC4X3/include/IfcQuantityCount.h>
#include <ifcpp/IFC4X3/include/IfcQuantityLength.h>
#include <ifcpp/IFC4X3/include/IfcQuantityVolume.h>
#include <ifcpp/IFC4X3/include/IfcQuantityWeight.h>
#include <ifcpp/IFC4X3/include/IfcQuantityTime.h>
#include <ifcpp/IFC4X3/include/IfcQuantityNumber.h>

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
#include <ifcpp/IFC4X3/include/IfcIlluminanceMeasure.h>
#include <ifcpp/IFC4X3/include/IfcPowerMeasure.h>
#include <ifcpp/IFC4X3/include/IfcVolumetricFlowRateMeasure.h>

#include <ifcpp/IFC4X3/include/IfcAreaMeasure.h>
#include <ifcpp/IFC4X3/include/IfcCountMeasure.h>
#include <ifcpp/IFC4X3/include/IfcLengthMeasure.h>
#include <ifcpp/IFC4X3/include/IfcPositiveRatioMeasure.h>
#include <ifcpp/IFC4X3/include/IfcRatioMeasure.h>
#include <ifcpp/IFC4X3/include/IfcThermodynamicTemperatureMeasure.h>
#include <ifcpp/IFC4X3/include/IfcTimeMeasure.h>
#include <ifcpp/IFC4X3/include/IfcVolumeMeasure.h>
#include <ifcpp/IFC4X3/include/IfcMassMeasure.h>

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
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
	else if(propSetName == "Pset_SpaceCommon") {
		if(propName == "Reference")
			return true;
		if(propName == "IsExternal")
			return true;
		if(propName == "GrossPlannedArea")
			return true;
		if(propName == "NetPlannedArea")
			return true;
	}
	else if(propSetName == "Pset_SpaceOccupancyRequirements") {
		if(propName == "OccupancyType")
			return true;
		if(propName == "OccupancyNumber")
			return true;
		if(propName == "OccupancyNumberPeak")
			return true;
		if(propName == "OccupancyTimePerDay")
			return true;
		if(propName == "AreaPerOccupant")
			return true;
	}
	else if(propSetName == "Pset_SpaceThermalRequirements") {
		return true;
	}
	else if(propSetName == "Pset_AirSideSystemInformation") {
		return true;
	}
	else if(propSetName == "Pset_SpaceThermalDesign") {
		return true;
	}
	else if(propSetName == "Pset_SpaceThermalLoad") {
		return true;
	}

	return false;
}

bool isSimpleProperty(shared_ptr<IFC4X3::IfcProperty> property) {
	shared_ptr<IFC4X3::IfcSimpleProperty> simpleValue = dynamic_pointer_cast<IFC4X3::IfcSimpleProperty>(property);
	if(simpleValue)
		return true;
	return false;
}

template<class T>
bool setDoubleProp(shared_ptr<IFC4X3::IfcPropertySingleValue> singleValue, Property& prop, std::string description = std::string()) {
	shared_ptr<T> value1 = dynamic_pointer_cast<T>(singleValue->m_NominalValue);
	if(value1) {
		prop.m_description = description;
		prop.m_valueType = Property::VT_Double;
		prop.m_doubleValue = value1->m_value;
		return true;
	}
	return false;
}

template<class T>
bool setQuantity(T singleValue, Property& prop, std::string description = std::string()) {
	if(singleValue) {
		prop.m_description = description;
		prop.m_valueType = Property::VT_Double;
		prop.m_doubleValue = singleValue->m_value;
		return true;
	}
	return false;
}

template<class T>
bool setBoundedProp(shared_ptr<IFC4X3::IfcPropertyBoundedValue> boundedValue, Property& prop, std::string description = std::string()) {
	prop.m_description = description;
	prop.m_valueType = Property::VT_Bounded;
	bool hasOne = false;
	shared_ptr<T> valueUp = dynamic_pointer_cast<T>(boundedValue->m_UpperBoundValue);
	if(valueUp) {
		prop.m_boundedValue.m_upperBound = valueUp->m_value;
		hasOne = true;
	}
	shared_ptr<T> valueDown = dynamic_pointer_cast<T>(boundedValue->m_LowerBoundValue);
	if(valueUp) {
		prop.m_boundedValue.m_lowerBound = valueDown->m_value;
		hasOne = true;
	}
	shared_ptr<T> valueSet = dynamic_pointer_cast<T>(boundedValue->m_SetPointValue);
	if(valueUp) {
		prop.m_boundedValue.m_setPoint = valueSet->m_value;
		hasOne = true;
	}
	return hasOne;
}

bool getProperty(shared_ptr<IFC4X3::IfcProperty> property, const std::string& pset_name, Property& prop) {

	if(!isSimpleProperty(property))
		return false;

	prop.m_setName = pset_name;

	shared_ptr<IFC4X3::IfcPropertyBoundedValue> boundedValue = dynamic_pointer_cast<IFC4X3::IfcPropertyBoundedValue>(property);
	if(boundedValue) {
		if(setBoundedProp<IFC4X3::IfcPowerMeasure>(boundedValue, prop, "Power"))
			return true;
	}
	shared_ptr<IFC4X3::IfcPropertyEnumeratedValue> enumeratedValue = dynamic_pointer_cast<IFC4X3::IfcPropertyEnumeratedValue>(property);
	shared_ptr<IFC4X3::IfcPropertyListValue> listValue = dynamic_pointer_cast<IFC4X3::IfcPropertyListValue>(property);
	shared_ptr<IFC4X3::IfcPropertyReferenceValue> referenceValue = dynamic_pointer_cast<IFC4X3::IfcPropertyReferenceValue>(property);
	shared_ptr<IFC4X3::IfcPropertySingleValue> singleValue = dynamic_pointer_cast<IFC4X3::IfcPropertySingleValue>(property);
	if(singleValue) {
		if(!singleValue->m_NominalValue)
			return false;

		shared_ptr<IFC4X3::IfcDerivedMeasureValue> dmv = dynamic_pointer_cast<IFC4X3::IfcDerivedMeasureValue>(singleValue->m_NominalValue);
		if(dmv) {
			if(setDoubleProp<IFC4X3::IfcHeatFluxDensityMeasure>(singleValue, prop, "HeatFlux"))
				return true;
			if(setDoubleProp<IFC4X3::IfcIsothermalMoistureCapacityMeasure>(singleValue, prop, "IsothermalMoistureCapacity"))
				return true;
			if(setDoubleProp<IFC4X3::IfcThermalConductivityMeasure>(singleValue, prop, "ThermalConductivity"))
				return true;
			if(setDoubleProp<IFC4X3::IfcThermalResistanceMeasure>(singleValue, prop, "ThermalResistance"))
				return true;
			if(setDoubleProp<IFC4X3::IfcThermalTransmittanceMeasure>(singleValue, prop, "ThermalTransmittance"))
				return true;
			if(setDoubleProp<IFC4X3::IfcVaporPermeabilityMeasure>(singleValue, prop, "VaporPermeability"))
				return true;
			if(setDoubleProp<IFC4X3::IfcIlluminanceMeasure>(singleValue, prop, "Illuminance"))
				return true;
			if(setDoubleProp<IFC4X3::IfcPowerMeasure>(singleValue, prop, "Power"))
				return true;
			if(setDoubleProp<IFC4X3::IfcVolumetricFlowRateMeasure>(singleValue, prop, "VolumetricFlowRate"))
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

//					IfcWarpingConstantMeasure
//					IfcWarpingMomentMeasure
		}
		shared_ptr<IFC4X3::IfcMeasureValue> mv = dynamic_pointer_cast<IFC4X3::IfcMeasureValue>(singleValue->m_NominalValue);
		if(mv) {
			if(setDoubleProp<IFC4X3::IfcAreaMeasure>(singleValue, prop, "Area"))
				return true;
			if(setDoubleProp<IFC4X3::IfcCountMeasure>(singleValue, prop, "Count"))
				return true;
			if(setDoubleProp<IFC4X3::IfcLengthMeasure>(singleValue, prop, "Length"))
				return true;
			if(setDoubleProp<IFC4X3::IfcPositiveRatioMeasure>(singleValue, prop, "PositiveRatio"))
				return true;
			if(setDoubleProp<IFC4X3::IfcRatioMeasure>(singleValue, prop, "Ratio"))
				return true;
			if(setDoubleProp<IFC4X3::IfcThermodynamicTemperatureMeasure>(singleValue, prop, "ThermodynamicTemperature"))
				return true;
			if(setDoubleProp<IFC4X3::IfcTimeMeasure>(singleValue, prop, "Time"))
				return true;
//			IfcAmountOfSubstanceMeasure
//
//			IfcComplexNumber
//			IfcContextDependentMeasure

//			IfcDescriptiveMeasure
//			IfcElectricCurrentMeasure

//			IfcLuminousIntensityMeasure
//			IfcMassMeasure
//			IfcNonNegativeLengthMeasure
//			IfcNormalisedRatioMeasure
//			IfcNumericMeasure
//			IfcParameterValue
//			IfcPlaneAngleMeasure
//			IfcPositiveLengthMeasure
//			IfcPositivePlaneAngleMeasure


//			IfcSolidAngleMeasure


//			IfcVolumeMeasure
		}
		shared_ptr<IFC4X3::IfcSimpleValue> sv = dynamic_pointer_cast<IFC4X3::IfcSimpleValue>(singleValue->m_NominalValue);
		if(sv) {
			if(dynamic_pointer_cast<IFC4X3::IfcBinary>(sv)) {
				prop.m_description = "Binary";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = dynamic_pointer_cast<IFC4X3::IfcBinary>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcBoolean>(sv)) {
				prop.m_description = "Boolean";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_Boolean;
				prop.m_boolValue = dynamic_pointer_cast<IFC4X3::IfcBoolean>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcDate>(sv)) {
				prop.m_description = "Date";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = dynamic_pointer_cast<IFC4X3::IfcIdentifier>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcDateTime>(sv)) {
				prop.m_description = "DateTime";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = dynamic_pointer_cast<IFC4X3::IfcIdentifier>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcDuration>(sv)) {
				prop.m_description = "Duration";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = dynamic_pointer_cast<IFC4X3::IfcIdentifier>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcIdentifier>(sv)) {
				prop.m_description = "Identifier";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = dynamic_pointer_cast<IFC4X3::IfcIdentifier>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcInteger>(sv)) {
				prop.m_description = "Integer";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_INT;
				prop.m_intValue = dynamic_pointer_cast<IFC4X3::IfcInteger>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcLabel>(sv)) {
				prop.m_description = "Label";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = label2s(dynamic_pointer_cast<IFC4X3::IfcLabel>(sv));
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcLogical>(sv)) {
				prop.m_description = "Logical";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_Boolean;
				prop.m_boolValue = dynamic_pointer_cast<IFC4X3::IfcLogical>(sv)->m_value == LOGICAL_TRUE;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcPositiveInteger>(sv)) {
				prop.m_description = "PositiveInteger";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_INT;
				prop.m_intValue = dynamic_pointer_cast<IFC4X3::IfcPositiveInteger>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcReal>(sv)) {
				prop.m_description = "Real";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_Double;
				prop.m_doubleValue = dynamic_pointer_cast<IFC4X3::IfcReal>(sv)->m_value;
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcText>(sv)) {
				prop.m_description = "Text";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = text2s(dynamic_pointer_cast<IFC4X3::IfcText>(sv));
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcTime>(sv)) {
				prop.m_description = "Time";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_String;
				prop.m_stringValue = text2s(dynamic_pointer_cast<IFC4X3::IfcText>(sv));
				return true;
			}
			else if(dynamic_pointer_cast<IFC4X3::IfcTimeStamp>(sv)) {
				prop.m_description = "TimeStamp";
				prop.m_name = singleValue->m_Name->m_value;
				prop.m_valueType = Property::VT_INT;
				prop.m_intValue = dynamic_pointer_cast<IFC4X3::IfcPositiveInteger>(sv)->m_value;
				return true;
			}
		}
	}
	shared_ptr<IFC4X3::IfcPropertyTableValue> tableValue = dynamic_pointer_cast<IFC4X3::IfcPropertyTableValue>(property);
	return false;
}

bool getQuantity(shared_ptr<IFC4X3::IfcPhysicalQuantity> quantity, const std::string& qset_name, Property& prop) {
	prop.m_setName = qset_name;
	prop.m_name = label2s(quantity->m_Name);
	shared_ptr<IFC4X3::IfcQuantityArea> qa = dynamic_pointer_cast<IFC4X3::IfcQuantityArea>(quantity);
	if(qa) {
		if(setQuantity(qa->m_AreaValue, prop, "Quantity Area"))
			return true;
	}
	shared_ptr<IFC4X3::IfcQuantityCount> qc = dynamic_pointer_cast<IFC4X3::IfcQuantityCount>(quantity);
	if(qc) {
		if(setQuantity(qc->m_CountValue, prop, "Quantity Count"))
			return true;
	}
	shared_ptr<IFC4X3::IfcQuantityLength> ql = dynamic_pointer_cast<IFC4X3::IfcQuantityLength>(quantity);
	if(ql) {
		if(setQuantity(ql->m_LengthValue, prop, "Quantity Length"))
			return true;
	}
	shared_ptr<IFC4X3::IfcQuantityTime> qt = dynamic_pointer_cast<IFC4X3::IfcQuantityTime>(quantity);
	if(qt) {
		if(setQuantity(qt->m_TimeValue, prop, "Quantity Time"))
			return true;
	}
	shared_ptr<IFC4X3::IfcQuantityVolume> qv = dynamic_pointer_cast<IFC4X3::IfcQuantityVolume>(quantity);
	if(qv) {
		if(setQuantity(qv->m_VolumeValue, prop, "Quantity Volume"))
			return true;
	}
	shared_ptr<IFC4X3::IfcQuantityWeight> qw = dynamic_pointer_cast<IFC4X3::IfcQuantityWeight>(quantity);
	if(qw) {
		if(setQuantity(qw->m_WeightValue, prop, "Quantity Weight"))
			return true;
	}
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

void getSpaceProperties(const shared_ptr<IFC4X3::IfcSpace>& space, std::map<std::string,std::map<std::string,Property>>& propItem) {
	for(const auto& relproperties : space->m_IsDefinedBy_inverse) {
		shared_ptr<IFC4X3::IfcRelDefinesByProperties> space_properties(relproperties);
		if(space_properties) {
			std::string pset_name = label2s(space_properties->m_Name);
			shared_ptr<IFC4X3::IfcPropertySetDefinition> propertySetDefinition = dynamic_pointer_cast<IFC4X3::IfcPropertySetDefinition>(space_properties->m_RelatingPropertyDefinition);
			if(propertySetDefinition) {
				pset_name = label2s(propertySetDefinition->m_Name);
				shared_ptr<IFC4X3::IfcElementQuantity> quantitySet = dynamic_pointer_cast<IFC4X3::IfcElementQuantity>(propertySetDefinition);
				if(quantitySet) {
					for(const auto& quantity : quantitySet->m_Quantities) {
						Property prop;
						getQuantity(quantity, pset_name, prop);
						std::map<std::string, Property> inner;
						inner.insert(std::make_pair(prop.m_name, prop));
						propItem.insert(std::make_pair(pset_name, inner));
					}
				}
				shared_ptr<IFC4X3::IfcPropertySet> propertySet = dynamic_pointer_cast<IFC4X3::IfcPropertySet>(propertySetDefinition);
				if(propertySet) {
					for(const auto& property : propertySet->m_HasProperties) {
						std::string name = name2s(property->m_Name);
						bool usesThisProperty = Property::relevantProperty(pset_name, name);
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
				shared_ptr<IFC4X3::IfcPreDefinedPropertySet> preDefinedPropertySet = dynamic_pointer_cast<IFC4X3::IfcPreDefinedPropertySet>(propertySetDefinition);
				if(preDefinedPropertySet) {

				}
			}
			shared_ptr<IFC4X3::IfcPropertySetDefinitionSet> propertySetDefinitionSet = dynamic_pointer_cast<IFC4X3::IfcPropertySetDefinitionSet>(space_properties->m_RelatingPropertyDefinition);
			if(propertySetDefinitionSet) {
				for(const auto& propSet : propertySetDefinitionSet->m_vec) {
					pset_name = label2s(propSet->m_Name);
					shared_ptr<IFC4X3::IfcElementQuantity> quantitySet = dynamic_pointer_cast<IFC4X3::IfcElementQuantity>(propertySetDefinition);
					if(quantitySet) {
						for(const auto& quantity : quantitySet->m_Quantities) {
							Property prop;
							getQuantity(quantity, pset_name, prop);
							std::map<std::string, Property> inner;
							inner.insert(std::make_pair(prop.m_name, prop));
							propItem.insert(std::make_pair(pset_name, inner));
						}
					}
					shared_ptr<IFC4X3::IfcPropertySet> propertySet = dynamic_pointer_cast<IFC4X3::IfcPropertySet>(propertySetDefinition);
					if(propertySet) {
						for(const auto& property : propertySet->m_HasProperties) {
							std::string name = name2s(property->m_Name);
							bool usesThisProperty = Property::relevantProperty(pset_name, name);
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
					shared_ptr<IFC4X3::IfcPreDefinedPropertySet> preDefinedPropertySet = dynamic_pointer_cast<IFC4X3::IfcPreDefinedPropertySet>(propertySetDefinition);
					if(preDefinedPropertySet) {

					}
				}
			}
		}
	}

}


} // namespace IFCC
