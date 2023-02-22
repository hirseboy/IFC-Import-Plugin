#ifndef IFCC_PropertyH
#define IFCC_PropertyH

#include <string>

#include <ifcpp/IFC4X3/include/IfcProperty.h>
#include <ifcpp/IFC4X3/include/IfcMaterial.h>



namespace IFCC {

class Property
{
public:
	enum ValueType {
		VT_String,
		VT_Double,
		VT_Boolean,
		VT_INT,
		VT_NotDefined
	};

	Property();

	static bool relevantProperty(const std::string& propSetName, const std::string& propName);

	std::string m_name;
	std::string m_description;
	ValueType	m_valueType;
	std::string m_stringValue;
	bool		m_boolValue;
	int			m_intValue;
	double		m_doubleValue;
};

bool getDoubleProperty(const std::map<std::string,std::map<std::string,Property>>& pmap,
								 const std::string& setName, const std::string& name, double& value );

bool getProperty(shared_ptr<IFC4X3::IfcProperty> property, const std::string& pset_name, Property& prop);

void getMaterialProperties(const shared_ptr<IFC4X3::IfcMaterial>& mat, std::map<std::string,std::map<std::string,Property>>& propItem);

} // namespace IFCC

#endif // IFCC_PropertyH
