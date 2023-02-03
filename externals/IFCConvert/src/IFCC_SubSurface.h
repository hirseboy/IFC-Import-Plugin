#ifndef IFCC_SubSurfaceH
#define IFCC_SubSurfaceH

#include <tinyxml.h>

#include <IBKMK_Vector2D.h>

#include "IFCC_GeometricHelperClasses.h"

namespace IFCC {

class Surface;

/*! Represents a subsurface in a surface.
	The subsurface is defined as 2D polygon in a plane (of parent surface).
*/
class SubSurface
{
public:
	/*! Constructor.
		Create plane from parent polygon and convert given 3D polygon into 2D one in this plane.
		The object is only valid if the given polygon is also valid.
		\param polygon Polygon of the subsurface
		\param parentSurface Parent surface in whose plane the polygon will lie
	*/
	SubSurface(const std::vector<IBKMK::Vector3D>& polygon, const Surface& parentSurface);

	/*! Initialize the subsurface.*/
	void set(int id, const std::string& name, int elementId);

	/*! Return the 2D polygon.*/
	const std::vector<IBKMK::Vector2D>& polygon() const {
		return m_polyVect;
	}

	/*! Return if the object is valid.*/
	bool isValid() const {
		return m_valid;
	}

	/*! Return the object id.*/
	int id() const {
		return m_id;
	}

	/*! Return the id of the corresponding building element.*/
	int elementId() const {
		return m_elementEntityId;
	}

	/*! Return the name of the subsurface.*/
	std::string name() const {
		return m_name;
	}

	/*! Flip the surfrace polygone.*/
	void flip();

	/*! Write the subsurface in vicus xml format.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

private:
	int										m_id;				///< Unique id of the object
	int										m_elementEntityId;	///< Id of the corresponding building element (opening)
	std::string								m_name;				///< Name of the subsurface
	bool									m_valid;			///< Validity of the object. Is set in constructor.
	std::vector<IBKMK::Vector2D>			m_polyVect;			///< 2D polygon of the subsurface
	PlaneNormal								m_planeNormal;		///< Plane in whose the polygone must lie
};

} // namespace IFCC

#endif // IFCC_SubSurface_H
