#ifndef IFCC_ClipperToolsH
#define IFCC_ClipperToolsH

#include <clipper.hpp>

#include <carve/matrix.hpp>

#include "IFCC_Types.h"

namespace IFCC {

class PlaneNormal;

/*! Contains results of function intersectPolygons2.
*/
struct IntersectionResult {

	/*! Result is only valid if at least one intersection exist and contains a valid polygon.*/
	bool isValid() const {
		if(m_intersections.empty())
			return false;
		for(const auto& poly : m_intersections)
			if(poly.size() > 2)
				return true;

		return false;
	}

	/*! Vector of intersection polygons.*/
	std::vector<polygon3D_t>				m_intersections;
	/*! Vector of polygons from operation 'BasePolygon - ClipPolygon'*/
	std::vector<polygon3D_t>				m_diffBaseMinusClip;
	/*! Vector of hole polygones for each existing diffBaseMinusClip-polygon.
		First dimension must be the same as m_diffBaseMinusClip.
	*/
	std::vector<std::vector<polygon3D_t>>	m_holesBaseMinusClip;
	/*! Vector of polygons from operation 'ClipPolygon - BasePolygon'*/
	std::vector<polygon3D_t>				m_diffClipMinusBase;
	/*! Vector of hole polygones for each existing diffClipMinusBase-polygon.
		First dimension must be the same as m_diffClipMinusBase.
	*/
	std::vector<std::vector<polygon3D_t>>	m_holesClipMinusBase;
};

/*! Polygon to_merge will be merged into polygon base.
	The resulting polygon will be returned.
	The function uses internally the clipper function. This includes a conversion of the given 3D polygons into 2D polygons of clipper path type.
	\param base Base polygon for merging
	\param to_merge Polygon for merging into base polygon
	\param plane Plane in 3D in normal form for internal 3D to 2D and back conversion.
*/
polygon3D_t mergePolygons(const polygon3D_t& base, const polygon3D_t& to_merge, const PlaneNormal& plane);

/*! Returns the intersection area of intersectPoly in base polygon.
	The function uses internally the clipper function. This includes a conversion of the given 3D polygons into 2D polygons of clipper path type.
	\param base Base polygon for calculation intersection.
	\param intersectPoly Polygon which intersects base polygon
	\param plane Plane in 3D in normal form for internal 3D to 2D and back conversion.
	\return Intersection polygon. It is not valid (empty) in case of no intersection exist.
*/
polygon3D_t intersectPolygons(const polygon3D_t& base, const polygon3D_t& intersectPoly, const PlaneNormal& plane);

/*! Calculates all polygons which can be calculated by intersection of intersectPoly into base.
	The result will contain the intersection polygon, the rest base polygon and the rest inetsect polygon including existing holes.
	\param base Base polygon for calculation intersection.
	\param intersectPoly Polygon which intersects base polygon
	\param plane Plane in 3D in normal form for internal 3D to 2D and back conversion.
	\return All resulting polygons with holes (if exists). \sa IntersectionResult
*/
IntersectionResult intersectPolygons2(const polygon3D_t& base, const polygon3D_t& intersectPoly, const PlaneNormal& plane);

} // namespace IFCC

#endif // IFCC_ClipperToolsH
