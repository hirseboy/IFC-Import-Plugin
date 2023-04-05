/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#pragma once
#include <iostream>
#include <sstream>
#include <map>
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingObject.h"
#include "IfcAppliedValueSelect.h"
#include "IfcMetricValueSelect.h"

namespace IFC4X3
{
	// TYPE IfcValue = SELECT	(IfcDerivedMeasureValue	,IfcMeasureValue	,IfcSimpleValue);
	class IFCQUERY_EXPORT IfcValue : public IfcAppliedValueSelect, public IfcMetricValueSelect
	{
	public:
		virtual void getStepParameter( std::stringstream& stream, bool is_select_type = false ) const = 0;
		static shared_ptr<IfcValue> createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream );
	};
}

