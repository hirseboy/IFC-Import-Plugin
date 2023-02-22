#ifndef IFCC_EntityBaseH
#define IFCC_EntityBaseH

#include <ifcpp/IFC4X3/include/IfcRoot.h>

namespace IFCC {

/*! Base class for all object classes in IFCC.
	Contains id, guid and name.
*/
class EntityBase
{
public:
	/*! Standard constructor.
		\param id Unique id.
	*/
	explicit EntityBase(int id);

	/*! Set name, description and guid from given IFC object.
		\param ifcObject Base IFC object.
	*/
	bool set(std::shared_ptr<IFC4X3::IfcRoot> ifcObject);

	std::string				m_name;			///< Object name
	std::string				m_description;	///< Object description
	std::string				m_guid;			///< GUID of the IFC object
	int						m_id;			///< Unique id.
};

} // namespace IFCC

#endif // IFCC_EntityBaseH
