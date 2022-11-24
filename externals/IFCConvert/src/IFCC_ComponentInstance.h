#ifndef IFCC_ComponentInstanceH
#define IFCC_ComponentInstanceH

#include <vector>
#include <map>

//#include <VICUS_ComponentInstance.h>
//#include <VICUS_SubSurfaceComponentInstance.h>

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

//	/*! Create a VICUS component instance object and return this.
//		The returned object contains all transferable data.
//		\param idMap Used for mapping current ids to VICUS ids.
//	*/
//	VICUS::ComponentInstance getVicusComponentInstance(std::map<int,int>& idMap) const;

//	/*! Create a VICUS component instance object and return this.
//		The returned object contains all transferable data.
//		\param idMap Used for mapping current ids to VICUS ids.
//	*/
//	VICUS::SubSurfaceComponentInstance getVicusSubSurfaceComponentInstance(std::map<int,int>& idMap) const;

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

private:

};


} // namespace IFCC

#endif // IFCC_ComponentInstance_H
