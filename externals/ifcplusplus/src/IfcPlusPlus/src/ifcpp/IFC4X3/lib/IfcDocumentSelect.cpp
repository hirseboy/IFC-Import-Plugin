/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <map>
#include "ifcpp/model/BasicTypes.h"
#include "ifcpp/model/BuildingException.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/IFC4X3/include/IfcDocumentSelect.h"

// TYPE IfcDocumentSelect = SELECT	(IfcDocumentInformation	,IfcDocumentReference);
shared_ptr<IFC4X3::IfcDocumentSelect> IFC4X3::IfcDocumentSelect::createObjectFromSTEP( const std::string& arg, const std::map<int,shared_ptr<BuildingEntity> >& map, std::stringstream& errorStream )
{
	if( arg.empty() ){ return shared_ptr<IfcDocumentSelect>(); }
	if( arg.compare("$")==0 )
	{
		return shared_ptr<IfcDocumentSelect>();
	}
	if( arg.compare("*")==0 )
	{
		return shared_ptr<IfcDocumentSelect>();
	}
	shared_ptr<IfcDocumentSelect> result_object;
	readSelectType( arg, result_object, map, errorStream );
	return result_object;
}
