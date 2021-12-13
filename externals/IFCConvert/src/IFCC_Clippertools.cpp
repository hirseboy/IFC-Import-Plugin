#include "IFCC_Clippertools.h"

#include <IBKMK_2DCalculations.h>
#include <IBKMK_3DCalculations.h>
#include <IBKMK_Polygon3D.h>

#include <IBK_assert.h>

namespace IFCC {

ClipperLib::Path toPath(const std::vector<IBKMK::Vector2D>& vect) {
	ClipperLib::Path res;
	for(const auto& point : vect) {
		res.push_back(ClipperLib::IntPoint(point.m_x * CONVERSION, point.m_y * CONVERSION));
	}
	return res;
}

polygon2D_t polygon2DFromPath(const ClipperLib::Path& path) {
	polygon2D_t res;
	for(const auto& point : path) {
		res.push_back(IBKMK::Vector2D(point.X * RE_CONVERSION, point.Y * RE_CONVERSION));
	}
	return res;
}

polygon3D_t polygon3DFromPath(const ClipperLib::Path& path, const PlaneNormal& plane) {
	polygon2D_t temp = polygon2DFromPath(path);
	return create3DFrom2D(temp, plane);
}

std::vector<polygon3D_t> polygons3DFromPaths(const ClipperLib::Paths& paths, const PlaneNormal& plane) {
	std::vector<polygon3D_t> res;
	for(const ClipperLib::Path& path : paths) {
		res.push_back(polygon3DFromPath(path, plane));
	}
	return res;
}


static bool samePoint(const IBKMK::Vector3D& p1, const IBKMK::Vector3D& p2) {
	return (p1.distanceTo(p2) < 1e-6);
}

static polygon2D_t selfPojectPolygon3D(const polygon3D_t& poly3D, IBKMK::Vector3D& pos, IBKMK::Vector3D& localX,
									   IBKMK::Vector3D& localY, bool checkValid) {
const double MIN_LENGHT = 1e-3;

	polygon2D_t res;
	IBKMK::Vector3D p1(poly3D.front());
	int point2Found = -1;
	IBKMK::Vector3D p2;
	for(size_t i=1; i<poly3D.size(); ++i) {
		if(!samePoint(p1, poly3D[i])) {
			point2Found = i;
			p2 = poly3D[i];
			break;
		}
	}
	if(point2Found == -1)
		return res;

	int point3Found = -1;
	IBKMK::Vector3D p3;
	for(size_t i=poly3D.size()-1; i>1; --i) {
		if(i != point2Found && !samePoint(p1, poly3D[i]) && !samePoint(p2, poly3D[i])) {
			IBKMK::Vector3D a1 = p2 - p1;
			IBKMK::Vector3D a2 = poly3D[i] - p1;
			IBKMK::Vector3D n;
			a1.crossProduct(a2, n);
			if (n.magnitude() > MIN_LENGHT) {
				point3Found = i;
				p3 = poly3D[i];
				break;
			}
		}
	}
	if(point3Found == -1)
		return res;

	pos = p1;
	localX = p2 - p1;
	IBKMK::Vector3D y = p3 - p1;
	IBKMK::Vector3D n;
	localX.crossProduct(y, n);
	n.normalize();

	int sameDirectionCount = 0;

	// now process all other points and generate their normal vectors as well
	for (unsigned int i=1; i<poly3D.size(); ++i) {
		IBKMK::Vector3D vx = poly3D[(i+1) % poly3D.size()] - poly3D[i];
		IBKMK::Vector3D vy = poly3D[i-1] - poly3D[i];
		IBKMK::Vector3D vn;
		vx.crossProduct(vy, vn);
		vn.normalize();
		// adding reference normal to current vertexes normal and checking magnitude works
		if ((vn + n).magnitude() > 1) // can be 0 or 2, so comparing against 1 is good even for rounding errors
			++sameDirectionCount;
		else
			--sameDirectionCount;
	}

	if (sameDirectionCount < 0) {
		// invert our normal vector
		n *= -1;
	}

	// now compute local Y axis
	n.crossProduct(localX, localY);
	// normalize localX and localY
	localX.normalize();
	localY.normalize();

	res.reserve(poly3D.size());
	for (unsigned int i=0; i<poly3D.size(); ++i) {
		const IBKMK::Vector3D & v = poly3D[i];
		double x = 0;
		double y = 0;
		if (!checkValid || IBKMK::planeCoordinates(pos, localX, localY, v, x, y)) {
			res.push_back( IBKMK::Vector2D(x,y) );
		}
		else {
			return res;
		}
	}
	return res;
}

static polygon2D_t projectPolygon3D(const polygon3D_t& poly3D, const IBKMK::Vector3D& pos, const IBKMK::Vector3D& localX,
									const IBKMK::Vector3D& localY, bool checkValid) {
	polygon2D_t res;
	res.reserve(poly3D.size());
	for (unsigned int i=0; i<poly3D.size(); ++i) {
		const IBKMK::Vector3D & v = poly3D[i];
		double x,y;
		if (!checkValid || IBKMK::planeCoordinates(pos, localX, localY, v, x, y)) {
			res.push_back( IBKMK::Vector2D(x,y) );
		}
		else {
			return res;
		}
	}
	return res;
}

polygon2D_t create2DFrom3D(const polygon3D_t& baseVect, const PlaneNormal& plane) {
	polygon2D_t polygon2DBase;
	for(const IBKMK::Vector3D& vect : baseVect) {
		IBKMK::Vector2D p2 = plane.convert3DPoint(vect);
		polygon2DBase.push_back(p2);
	}

	return polygon2DBase;
}

conversionVectors2D_t create2DFrom3D(const polygon3D_t& baseVect, const polygon3D_t& secVec, const PlaneNormal& plane) {
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

std::pair<ClipperLib::Path,ClipperLib::Path> createPathFrom3D(const polygon3D_t& baseVect, const polygon3D_t& secVec, const PlaneNormal& plane) {
	std::pair<ClipperLib::Path,ClipperLib::Path> result;

	std::pair<std::vector<IBKMK::Vector2D>,std::vector<IBKMK::Vector2D>> tmp = create2DFrom3D(baseVect, secVec, plane);

	result.first = toPath(tmp.first);
	result.second = toPath(tmp.second);
	return result;
}

//IBKMK::Vector3D conversion2DPoint( const IBKMK::Vector2D& point, const IBKMK::Vector3D& translate, IBKMK::Vector3D local_z, IBKMK::Vector3D local_x) {
//	IBKMK::Vector3D  local_y(  0.0, 1.0, 0.0 );

//	local_y = local_z.crossProduct(local_x );
//	// local_x can be just in the x-z-plane, not perpendicular to y and z. so re-compute local x
//	local_x = local_y.crossProduct( local_z );

//	local_x.normalize();
//	local_y.normalize();
//	local_z.normalize();

//	double x = local_x.m_x * point.m_x + local_y.m_x * point.m_y + translate.m_x;
//	double y = local_x.m_y * point.m_x + local_y.m_y * point.m_y + translate.m_y;
//	double z = local_x.m_z * point.m_x + local_y.m_z * point.m_y + translate.m_z;

//	IBKMK::Vector3D res = IBKMK::Vector3D(x, y, z);
//	return res;
//}

polygon3D_t create3DFrom2D(const polygon2D_t& poly2, const PlaneNormal& plane) {
	polygon3D_t res;
	for(const auto& point : poly2) {
		res.emplace_back(plane.convert3DPointInv(point));
	}
	return res;
}


//IBKMK::Vector3D conversion3DPoint( const IBKMK::Vector3D& point, const IBKMK::Vector3D& translate, IBKMK::Vector3D local_z,
//								   IBKMK::Vector3D local_x) {
//	IBKMK::Vector3D  local_y = local_z.crossProduct(local_x );
//	// local_x can be just in the x-z-plane, not perpendicular to y and z. so re-compute local x
//	local_x = local_y.crossProduct( local_z );

//	local_x.normalize();
//	local_y.normalize();
//	local_z.normalize();

//	double x = local_x.m_x * point.m_x + local_y.m_x * point.m_y + local_z.m_x * point.m_z + translate.m_x;
//	double y = local_x.m_y * point.m_x + local_y.m_y * point.m_y + local_z.m_y * point.m_z + translate.m_y;
//	double z = local_x.m_z * point.m_x + local_y.m_z * point.m_y + local_z.m_z * point.m_z + translate.m_z;

//	IBKMK::Vector3D res = IBKMK::Vector3D(x, y, z);
//	return res;
//}

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
			for(int j=0; j<childCount; ++j) {
				ClipperLib::PolyNode* holeNode = node->Childs[i];
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
