#ifndef IFCC_ClipperToolsH
#define IFCC_ClipperToolsH

#include <clipper.hpp>

#include <carve/matrix.hpp>

#include "IFCC_Types.h"
#include "IFCC_GeometricHelperClasses.h"

namespace IFCC {

const ClipperLib::cInt CONVERSION = 1e5;
const double RE_CONVERSION = 1.0e-5;

struct IntersectionResult {

	bool isValid() const {
		if(m_intersections.empty())
			return false;
		for(const auto& poly : m_intersections)
			if(poly.size() > 2)
				return true;

		return false;
	}

	std::vector<polygon3D_t>				m_intersections;
	std::vector<polygon3D_t>				m_diffBaseMinusClip;
	std::vector<std::vector<polygon3D_t>>	m_holesBaseMinusClip;
	std::vector<polygon3D_t>				m_diffClipMinusBase;
	std::vector<std::vector<polygon3D_t>>	m_holesClipMinusBase;
};

ClipperLib::Path toPath(const polygon2D_t& vect);

polygon2D_t polygon2DFromPath(const ClipperLib::Path& path);

polygon3D_t polygon3DFromPath(const ClipperLib::Path& path, const PlaneNormal& plane);

std::vector<polygon3D_t> polygons3DFromPaths(const ClipperLib::Paths& paths, const PlaneNormal& plane);

polygon2D_t create2DFrom3D(const polygon3D_t& baseVect, const PlaneNormal& plane);

conversionVectors2D_t create2DFrom3D(const polygon3D_t& baseVect, const polygon3D_t& secVec, const PlaneNormal& plane);

std::pair<ClipperLib::Path,ClipperLib::Path> createPathFrom3D(const polygon3D_t& baseVect,
															const polygon3D_t& secVec, const PlaneNormal& plane);

polygon3D_t create3DFrom2D(const polygon2D_t& poly2, const PlaneNormal& plane);

IBKMK::Vector3D conversion2DPoint( const IBKMK::Vector2D& point, const IBKMK::Vector3D& translate, IBKMK::Vector3D local_z, IBKMK::Vector3D local_x);

IBKMK::Vector3D conversion3DPoint( const IBKMK::Vector3D& point, const IBKMK::Vector3D& translate, IBKMK::Vector3D local_z, IBKMK::Vector3D local_x);

/*! Polygon to_merge will be merged into polygon base.
	The resultung polygon will be returned.
*/
polygon3D_t mergePolygons(const polygon3D_t& base, const polygon3D_t& to_merge, const PlaneNormal& plane);

/*! Returns the intersection area of intersectPoly in base polygon.*/
polygon3D_t intersectPolygons(const polygon3D_t& base, const polygon3D_t& intersectPoly, const PlaneNormal& plane);

/*! Returns the intersection area in first and thew rest in second of intersectPoly in base polygon.*/
IntersectionResult intersectPolygons2(const polygon3D_t& base, const polygon3D_t& intersectPoly, const PlaneNormal& plane);

} // namespace IFCC

#endif // IFCC_ClipperToolsH
