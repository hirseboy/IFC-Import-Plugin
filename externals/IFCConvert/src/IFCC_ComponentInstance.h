#ifndef IFCC_ComponentInstanceH
#define IFCC_ComponentInstanceH

#include <vector>

#include <IBK_math.h>

#include <tinyxml.h>

namespace IFCC {

class ComponentInstance
{
public:

	ComponentInstance();

	int									m_id;
	int									m_componentId;
	int									m_sideASurfaceId;
	int									m_sideBSurfaceId;
	bool								m_subSurface;


	TiXmlElement * writeXML(TiXmlElement * parent) const;

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
