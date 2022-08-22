#include "IFCC_Clippertools.h"

#include <IBKMK_2DCalculations.h>
#include <IBKMK_3DCalculations.h>
#include <IBKMK_Polygon3D.h>

#include <IBK_assert.h>

#include "IFCC_GeometricHelperClasses.h"

namespace IFCC {

static const ClipperLib::cInt CONVERSION = 1e5;			///< Conversion factor for transforming double coordinates to int (0.1mm accuracy)
static const double RE_CONVERSION = 1.0 / CONVERSION;	///< Conversion factor for transforming int coordinates back to double


/*! Create a 3D polygon from the given 2D polygon which lies on the given plane.
	\param poly2 2D polygon for conversion into 3D.
	\param plane Plane in 3D in normal form for creating 3D polygon from 2D polygon.
	\return 3D polygon which lies in the given plane.
*/
static polygon3D_t create3DFrom2D(const polygon2D_t& poly2, const PlaneNormal& plane) {
	polygon3D_t res;
	for(const auto& point : poly2) {
		res.emplace_back(plane.convert3DPointInv(point));
	}
	return res;
}

/*! Converts 2D polygon into ClipperLib::Path type.
	This includes a conversion of double coordinats from original polygon into int coordinates of clipper path using CONVERSION factor.
	\param vect 2D polygon as vector of 2D points.
	\return 2D polygon as clipper path
*/
static ClipperLib::Path toPath(const std::vector<IBKMK::Vector2D>& vect) {
	ClipperLib::Path res;
	for(const auto& point : vect) {
		res.push_back(ClipperLib::IntPoint(point.m_x * CONVERSION, point.m_y * CONVERSION));
	}
	return res;
}

/*! Converts ClipperLib::Path into 2D polygon type.
	This includes a conversion of int coordinats from clipper path into double coordinates of 2D polygon using RE_CONVERSION factor.
	\param path 2D polygon as clipper path.
	\return 2D polygon as vector of 2D points.
*/
static polygon2D_t polygon2DFromPath(const ClipperLib::Path& path) {
	polygon2D_t res;
	for(const auto& point : path) {
		res.push_back(IBKMK::Vector2D(point.X * RE_CONVERSION, point.Y * RE_CONVERSION));
	}
	return res;
}

/*! Convert first clipper path into 2D polygon and from this create a 3D polygon which lies in the given plane.
	\param path 2D polygon as clipper path
	\param plane Plane in 3D in normal form for creating 3D polygon from 2D polygon.
	\return 3D polygon as vector of 3D points
*/
static polygon3D_t polygon3DFromPath(const ClipperLib::Path& path, const PlaneNormal& plane) {
	polygon2D_t temp = polygon2DFromPath(path);
	return create3DFrom2D(temp, plane);
}

/*! Convert first all clipper paths into 2D polygons and from this create a 3D polygons which lies in the given plane.
	\param paths 2D polygons as clipper path vector
	\param plane Plane in 3D in normal form for creating 3D polygon from 2D polygon.
	\return Vector of 3D polygones
*/
static std::vector<polygon3D_t> polygons3DFromPaths(const ClipperLib::Paths& paths, const PlaneNormal& plane) {
	std::vector<polygon3D_t> res;
	for(const ClipperLib::Path& path : paths) {
		res.push_back(polygon3DFromPath(path, plane));
	}
	return res;
}

/*! Check if two points are nearly the same by checking distance.*/
static bool samePoint(const IBKMK::Vector3D& p1, const IBKMK::Vector3D& p2) {
	return (p1.distanceTo(p2) < 1e-6);
}

/*! Create a 2D polygon from the given 3D polygon which lies on the given plane.
	\param baseVect 3D polygon for conversion into 2D.
	\param plane Plane in 3D in normal form for creating 2D polygon from 3D polygon.
	\return 2D polygon which lies in the given plane.
*/
static polygon2D_t create2DFrom3D(const polygon3D_t& baseVect, const PlaneNormal& plane) {
	polygon2D_t polygon2DBase;
	for(const IBKMK::Vector3D& vect : baseVect) {
		IBKMK::Vector2D p2 = plane.convert3DPoint(vect);
		polygon2DBase.push_back(p2);
	}

	return polygon2DBase;
}

/*! Create two 2D polygons from the given two 3D polygons which lies on the given plane.
	For correct conversion both 3D polygons must be plane parallel.
	\param baseVect First 3D polygon for conversion into 2D.
	\param secVec Second 3D polygon for conversion into 2D.
	\param plane Plane in 3D in normal form for creating 2D polygon from 3D polygon.
	\return Pair of two 2D polygons which lies in the given plane.
*/
static conversionVectors2D_t create2DFrom3D(const polygon3D_t& baseVect, const polygon3D_t& secVec, const PlaneNormal& plane) {
	conversionVectors2D_t result;

	polygon2D_t polygon2DBase;
	for(const IBKMK::Vector3D& vect : baseVect) {
		IBKMK::Vector2D p2 = plane.convert3DPoint(vect);
		polygon2DBase.push_back(p2);
	}
	if(polygon2DBase.empty())
		return result;

	polygon2D_t polygon2DSec;
	for(const IBKMK::Vector3D& vect : secVec) {
		IBKMK::Vector2D p2 = plane.convert3DPoint(vect);
		polygon2DSec.push_back(p2);
	}
	if(polygon2DSec.empty())
		return result;

	result.first = polygon2DBase;
	result.second = polygon2DSec;
	return result;
}

/*! Create two 2D polygons in clipper path type from the given two 3D polygons which lies on the given plane.
	For correct conversion both 3D polygons must be plane parallel.
	Internal conversion from double into int coordinates by using CONVERSION.
	\param baseVect First 3D polygon for conversion into 2D.
	\param secVec Second 3D polygon for conversion into 2D.
	\param plane Plane in 3D in normal form for creating 2D polygon from 3D polygon.
	\return Pair of two 2D polygons in clipper type which lies in the given plane.
*/
static std::pair<ClipperLib::Path,ClipperLib::Path> createPathFrom3D(const polygon3D_t& baseVect, const polygon3D_t& secVec, const PlaneNormal& plane) {
	std::pair<ClipperLib::Path,ClipperLib::Path> result;

	std::pair<std::vector<IBKMK::Vector2D>,std::vector<IBKMK::Vector2D>> tmp = create2DFrom3D(baseVect, secVec, plane);

	result.first = toPath(tmp.first);
	result.second = toPath(tmp.second);
	return result;
}

polygon3D_t mergePolygons(const polygon3D_t& base, const polygon3D_t& to_merge, const PlaneNormal& plane) {
	std::pair<ClipperLib::Path,ClipperLib::Path> clipPolys = createPathFrom3D(base, to_merge, plane);
	if(clipPolys.first.empty() || clipPolys.second.empty())
		return polygon3D_t();

	try {
		ClipperLib::Clipper clipper;
		clipper.AddPath(clipPolys.first, ClipperLib::ptSubject, true);
		clipper.AddPath(clipPolys.second, ClipperLib::ptClip, true);
		ClipperLib::Paths clipresult;
		clipper.Execute(ClipperLib::ctUnion, clipresult, ClipperLib::pftEvenOdd, ClipperLib::pftNonZero);
		std::vector<polygon3D_t> result = polygons3DFromPaths(clipresult, plane);
		if(result.empty())
			return polygon3D_t();

		return result.front();
	}
	catch (std::exception& e) {
		return polygon3D_t();
	}
	catch (...) {
		return polygon3D_t();
	}

}

polygon3D_t intersectPolygons(const polygon3D_t& base, const polygon3D_t& intersectPoly, const PlaneNormal& plane) {
	std::pair<ClipperLib::Path,ClipperLib::Path> clipPolys = createPathFrom3D(base, intersectPoly, plane);
	if(clipPolys.first.empty() || clipPolys.second.empty())
		return polygon3D_t();

	try {
		ClipperLib::Clipper clipper;
		clipper.AddPath(clipPolys.first, ClipperLib::ptSubject, true);
		clipper.AddPath(clipPolys.second, ClipperLib::ptClip, true);
		ClipperLib::Paths clipresult;
		clipper.Execute(ClipperLib::ctIntersection, clipresult, ClipperLib::pftEvenOdd, ClipperLib::pftNonZero);
		std::vector<polygon3D_t> result = polygons3DFromPaths(clipresult, plane);
		if(result.empty())
			return polygon3D_t();

		return result.front();
	}
	catch (std::exception& e) {
		return polygon3D_t();
	}
	catch (...) {
		return polygon3D_t();
	}

}

bool containHoles(const ClipperLib::PolyNode* node) {
	if(node == nullptr)
		return false;
	if(node->IsHole())
		return true;
	for(int i=0; i<node->ChildCount(); ++i) {
		if(containHoles(node->Childs[i]))
			return true;
	}
	if(containHoles(node->GetNext()))
		return true;
	return false;
}

bool containHoles(const ClipperLib::PolyTree& polytree) {
	if(polytree.Total() < 2)
		return false;
	ClipperLib::PolyNode* node = polytree.GetFirst();
	return containHoles(node);
}

static void dividePolyTree(const ClipperLib::PolyTree& polytree, std::vector<polygon3D_t>& outerPolygons, std::vector<std::vector<polygon3D_t>>& holes,
						   const PlaneNormal& plane) {
	int childCount = polytree.ChildCount();
	std::vector<polygon3D_t>(childCount).swap(outerPolygons);
	std::vector<std::vector<polygon3D_t>>(childCount).swap(holes);

	for(int i=0; i<childCount; ++i) {
		ClipperLib::PolyNode* node = polytree.Childs[i];
		IBK_ASSERT(node != nullptr);
		IBK_ASSERT(!node->IsHole());
		outerPolygons[i] = polygon3DFromPath(node->Contour, plane);
		// collect holes
		if(node->ChildCount() > 0) {
			for(int j=0; j<node->ChildCount(); ++j) {
				ClipperLib::PolyNode* holeNode = node->Childs[j];
				IBK_ASSERT(holeNode != nullptr);
				if(holeNode->IsHole())
					holes[i].push_back(polygon3DFromPath(holeNode->Contour, plane));

				// should not exist
				if(holeNode->ChildCount() > 0) {

				}
			}
		}
	}
}

IntersectionResult intersectPolygons2(const polygon3D_t& base, const polygon3D_t& intersectPoly, const PlaneNormal& plane) {

	std::pair<ClipperLib::Path,ClipperLib::Path> clipPolys = createPathFrom3D(base, intersectPoly, plane);
	if(clipPolys.first.empty() || clipPolys.second.empty())
		return IntersectionResult();

	try {
		IntersectionResult result;
		ClipperLib::Clipper clipper;
		clipper.AddPath(clipPolys.first, ClipperLib::ptSubject, true);
		clipper.AddPath(clipPolys.second, ClipperLib::ptClip, true);
		ClipperLib::PolyTree clipresult;
		clipper.Execute(ClipperLib::ctIntersection, clipresult, ClipperLib::pftEvenOdd, ClipperLib::pftNonZero);
		if(!containHoles(clipresult)) {
			ClipperLib::Paths closedPaths;
			ClipperLib::ClosedPathsFromPolyTree(clipresult, closedPaths);
			std::vector<polygon3D_t> tr1 = polygons3DFromPaths(closedPaths, plane);
			if(tr1.empty())
				return IntersectionResult();

			result.m_intersections = tr1;
		}
		// a intersection should not contain holes
		else {
			return IntersectionResult();
		}

		clipresult.Clear();
		clipper.Execute(ClipperLib::ctDifference, clipresult, ClipperLib::pftEvenOdd, ClipperLib::pftNonZero);
		std::vector<polygon3D_t> outerPolygonsA;
		std::vector<std::vector<polygon3D_t>> holesA;
		dividePolyTree(clipresult, outerPolygonsA, holesA, plane);
		result.m_diffBaseMinusClip = outerPolygonsA;
		result.m_holesBaseMinusClip = holesA;

		clipresult.Clear();
		clipper.Clear();
		clipper.AddPath(clipPolys.second, ClipperLib::ptSubject, true);
		clipper.AddPath(clipPolys.first, ClipperLib::ptClip, true);
		clipper.Execute(ClipperLib::ctDifference, clipresult, ClipperLib::pftEvenOdd, ClipperLib::pftNonZero);
		std::vector<polygon3D_t> outerPolygonsB;
		std::vector<std::vector<polygon3D_t>> holesB;
		dividePolyTree(clipresult, outerPolygonsB, holesB, plane);
		result.m_diffClipMinusBase = outerPolygonsB;
		result.m_holesClipMinusBase = holesB;

		return result;
	}
	catch (std::exception& e) {
		return IntersectionResult();
	}
	catch (...) {
		return IntersectionResult();
	}

}


} // namespace IFCC
