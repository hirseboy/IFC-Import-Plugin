#include "IFCC_EntityBase.h"

#include <codecvt>

#include <ifcpp/IFC4X3/include/IfcGloballyUniqueId.h>

#include <Carve/src/include/carve/carve.hpp>

#include "IFCC_MeshUtils.h"
#include "IFCC_Helper.h"

namespace IFCC {

EntityBase::EntityBase(int id) :
	m_id(id)
{
}

bool EntityBase::set(std::shared_ptr<IFC4X3::IfcRoot> ifcObject) {
	if(ifcObject == nullptr)
		return false;

	m_name = label2s(ifcObject->m_Name);
	m_description = text2s(ifcObject->m_Description);
//	std::wstring_convert<std::codecvt_utf8<char>, char> converterX;
//	m_guid = converterX.to_bytes(ifcObject->m_GlobalId->m_value);
	m_guid = ifcObject->m_GlobalId->m_value;

	return true;
}

} // namespace IFCC
