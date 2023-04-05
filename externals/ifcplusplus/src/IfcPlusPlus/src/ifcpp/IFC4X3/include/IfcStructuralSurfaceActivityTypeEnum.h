/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */

#pragma once
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include "ifcpp/model/GlobalDefines.h"
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingObject.h"

namespace IFC4X3
{
	// TYPE IfcStructuralSurfaceActivityTypeEnum = ENUMERATION OF	(BILINEAR	,CONST	,DISCRETE	,ISOCONTOUR	,USERDEFINED	,NOTDEFINED);
	class IFCQUERY_EXPORT IfcStructuralSurfaceActivityTypeEnum : virtual public BuildingObject
	{
	public:
		enum IfcStructuralSurfaceActivityTypeEnumEnum
		{
			ENUM_BILINEAR,
			ENUM_CONST,
			ENUM_DISCRETE,
			ENUM_ISOCONTOUR,
			ENUM_USERDEFINED,
			ENUM_NOTDEFINED
		};

		IfcStructuralSurfaceActivityTypeEnum() = default;
		IfcStructuralSurfaceActivityTypeEnum( IfcStructuralSurfaceActivityTypeEnumEnum e ) { m_enum = e; }
		virtual uint32_t classID() const { return 2860242611; }
		virtual void getStepParameter( std::stringstream& stream, bool is_select_type = false ) const;
		static shared_ptr<IfcStructuralSurfaceActivityTypeEnum> createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream );
		IfcStructuralSurfaceActivityTypeEnumEnum m_enum;
	};
}
