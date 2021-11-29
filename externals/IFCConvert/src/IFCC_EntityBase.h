#ifndef IFCC_EntityBaseH
#define IFCC_EntityBaseH

#include <ifcpp/IFC4/include/IfcRoot.h>

namespace IFCC {

class EntityBase
{
public:
	explicit EntityBase(int id);

	bool set(std::shared_ptr<IfcRoot> ifcObject);

	std::string				m_name;
	std::string				m_description;
	std::string				m_guid;
	int						m_id;
};

} // namespace IFCC

#endif // IFCC_EntityBaseH
