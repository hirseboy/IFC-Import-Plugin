#ifndef IFCC_SubSurfaceH
#define IFCC_SubSurfaceH

#include <carve/mesh.hpp>

#include <tinyxml.h>

#include "IFCC_Types.h"
#include "IFCC_Helper.h"
#include "IFCC_GeometricHelperClasses.h"

namespace IFCC {

class Surface;

class SubSurface
{
public:
	SubSurface(const std::vector<IBKMK::Vector3D>& polygon, const Surface& parentSurface, const IBKMK::Vector3D& nullPos);

	const PlaneNormal& planeNormal() const {
		return m_planeNormal;
	}

	const std::vector<IBKMK::Vector2D>& polygon() const {
		return m_polyVect;
	}

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	std::string								m_name;
	int										m_id;
	int										m_elementEntityId;
	int										m_openingId;
	bool									m_valid;

private:
	std::vector<IBKMK::Vector2D>			m_polyVect;
	PlaneNormal								m_planeNormal;
};

} // namespace IFCC

#endif // IFCC_SubSurface_H
