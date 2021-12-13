#ifndef IFCC_ClipperToolsH
#define IFCC_ClipperToolsH

#include <clipper.hpp>

#include <carve/matrix.hpp>

#include "IFCC_Types.h"
#include "IFCC_GeometricHelperClasses.h"

namespace IFCC {

const ClipperLib::cInt CONVERSION = 1e5;	///< Conversion factor for transforming double coordinates to int (0.1mm accuracy)
const double RE_CONVERSION = 1.0e-5;		///< Conversion factor for transforming int coordinates back to double

/*! Contains result of function intersectPolygons2.
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

/*! Converts 2D polygon into ClipperLib::Path type.
	This includes a conversion of double coordinats from original polygon into int coordinates of clipper path using CONVERSION factor.
	\param vect 2D polygon as vector of 2D points.
	\return 2D polygon as clipper path
*/
ClipperLib::Path toPath(const polygon2D_t& vect);

/*! Converts ClipperLib::Path into 2D polygon type.
	This includes a conversion of int coordinats from clipper path into double coordinates of 2D polygon using RE_CONVERSION factor.
	\param path 2D polygon as clipper path.
	\return 2D polygon as vector of 2D points.
*/
polygon2D_t polygon2DFromPath(const ClipperLib::Path& path);

/*! Convert first clipper path into 2D polygon and from this create a 3D polygon which lies in the given plane.
	\param path 2D polygon as clipper path
	\param plane Plane in 3D in normal form for creating 3D polygon from 2D polygon.
	\return 3D polygon as vector of 3D points
*/
polygon3D_t polygon3DFromPath(const ClipperLib::Path& path, const PlaneNormal& plane);

/*! Convert first all clipper paths into 2D polygons and from this create a 3D polygons which lies in the given plane.
	\param paths 2D polygons as clipper path vector
	\param plane Plane in 3D in normal form for creating 3D polygon from 2D polygon.
	\return Vector of 3D polygones
*/
std::vector<polygon3D_t> polygons3DFromPaths(const ClipperLib::Paths& paths, const PlaneNormal& plane);

/*! Create a 2D polygon from the given 3D polygon which lies on the given plane.
	\param baseVect 3D polygon for conversion into 2D.
	\param plane Plane in 3D in normal form for creating 2D polygon from 3D polygon.
	\return 2D polygon which lies in the given plane.
*/
polygon2D_t create2DFrom3D(const polygon3D_t& baseVect, const PlaneNormal& plane);

/*! Create two 2D polygons from the given two 3D polygons which lies on the given plane.
	For correct conversion both 3D polygons must be plane parallel.
	\param baseVect First 3D polygon for conversion into 2D.
	\param secVec Second 3D polygon for conversion into 2D.
	\param plane Plane in 3D in normal form for creating 2D polygon from 3D polygon.
	\return Pair of two 2D polygons which lies in the given plane.
*/
conversionVectors2D_t create2DFrom3D(const polygon3D_t& baseVect, const polygon3D_t& secVec, const PlaneNormal& plane);

/*! Create two 2D polygons in clipper path type from the given two 3D polygons which lies on the given plane.
	For correct conversion both 3D polygons must be plane parallel.
	Internal conversion from double into int coordinates by using CONVERSION.
	\param baseVect First 3D polygon for conversion into 2D.
	\param secVec Second 3D polygon for conversion into 2D.
	\param plane Plane in 3D in normal form for creating 2D polygon from 3D polygon.
	\return Pair of two 2D polygons in clipper type which lies in the given plane.
*/
std::pair<ClipperLib::Path,ClipperLib::Path> createPathFrom3D(const polygon3D_t& baseVect,
															const polygon3D_t& secVec, const PlaneNormal& plane);

/*! Create a 3D polygon from the given 2D polygon which lies on the given plane.
	\param poly2 2D polygon for conversion into 3D.
	\param plane Plane in 3D in normal form for creating 3D polygon from 2D polygon.
	\return 3D polygon which lies in the given plane.
*/
polygon3D_t create3DFrom2D(const polygon2D_t& poly2, const PlaneNormal& plane);

//IBKMK::Vector3D conversion2DPoint( const IBKMK::Vector2D& point, const IBKMK::Vector3D& translate, IBKMK::Vector3D local_z, IBKMK::Vector3D local_x);

//IBKMK::Vector3D conversion3DPoint( const IBKMK::Vector3D& point, const IBKMK::Vector3D& translate, IBKMK::Vector3D local_z, IBKMK::Vector3D local_x);

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
