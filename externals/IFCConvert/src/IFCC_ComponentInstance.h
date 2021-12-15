#ifndef IFCC_ComponentInstanceH
#define IFCC_ComponentInstanceH

#include <vector>

class TiXmlElement;

namespace IFCC {

/*! Class contains all informations for a component instance.
	A component instance connects a component with up to two surfaces.
	The component instance classe can be a normal component instance (construction surface) or a subsurface component instance (opening surface).
*/
class ComponentInstance {
public:

	/*! Standard constructor.*/
	ComponentInstance();

	int									m_id;				///< Id of the object
	int									m_componentId;		///< Id of the connected component
	int									m_sideASurfaceId;	///< Id of a surface connected to side A
	int									m_sideBSurfaceId;	///< Id of a surface connected to side B
	bool								m_subSurface;		///< If true its a subsurface component instance, otherwise its a normal component instance

	/*! Write the component in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Comparison operator.*/
	friend bool operator==(const ComponentInstance& left, const ComponentInstance& right) {
		if(left.m_componentId != right.m_componentId)
			return false;
		if(left.m_sideASurfaceId != right.m_sideASurfaceId)
			return false;
		if(left.m_sideBSurfaceId != right.m_sideBSurfaceId)
			return false;
		return true;
	}
};


} // namespace IFCC

#endif // IFCC_ComponentInstance_H
