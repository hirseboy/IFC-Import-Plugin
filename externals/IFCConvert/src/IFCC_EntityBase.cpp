#include "IFCC_EntityBase.h"

#include <codecvt>

#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>

#include "IFCC_Helper.h"

namespace IFCC {

EntityBase::EntityBase(int id) :
	m_id(id)
{
}

bool EntityBase::set(std::shared_ptr<IfcRoot> ifcObject) {
	if(ifcObject == nullptr)
		return false;

	m_name = label2s(ifcObject->m_Name);
	m_description = text2s(ifcObject->m_Description);
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	m_guid = converterX.to_bytes(ifcObject->m_GlobalId->m_value);

	return true;
}

} // namespace IFCC
