#ifndef IFCC_SurfaceH
#define IFCC_SurfaceH

#include <carve/mesh.hpp>

#include <tinyxml.h>

#include "IFCC_Types.h"
#include "IFCC_Helper.h"
#include "IFCC_GeometricHelperClasses.h"
#include "IFCC_SubSurface.h"

namespace IFCC {

class Surface
{
public:
	enum PositionType {
		PT_Internal,
		PT_External,
		PT_External_Ground,
		PT_Unknown
	};

	struct IntersectionResult;

	struct MatchResult {
		MatchResult() :
			m_wallSurfaceIndex(-1),
			m_spaceSurfaceIndex(-1)
		{}

		MatchResult(int wallSurfaceIndex, int spaceSurfaceIndex) :
			m_wallSurfaceIndex(wallSurfaceIndex),
			m_spaceSurfaceIndex(spaceSurfaceIndex)
		{}

		bool isValid() const {
			return m_wallSurfaceIndex > -1 && m_spaceSurfaceIndex > -1;
		}

		int m_wallSurfaceIndex;
		int m_spaceSurfaceIndex;
	};

	Surface();

	Surface(carve::mesh::Face<3>* face);

	Surface(const polygon3D_t& polygon);

	const carve::geom::plane<3>& planeCarve() const {
		return m_planeCarve;
	}

	const PlaneNormal& planeNormal() const {
		return m_planeNormal;
	}

	IBKMK::Vector3D centroid() const;

	double distancePointToPlane(const IBKMK::Vector3D& point, bool negate) const;

	double distanceToParallelPlane(const carve::geom::plane<3>& other) const;

	double distanceToParallelPlane(const PlaneNormal& other) const;

	bool isParallelTo(const Surface& other) const;

	bool equalNormals(const Surface& other)  const;

	bool equalNegNormals(const Surface& other)  const;

	const std::vector<IBKMK::Vector3D>& polygon() const {
		return m_polyVect;
	}

	void simplify();

	/*! Return true if the given surface intersects the original one.*/
	bool isIntersected(const Surface& other) const;

	/*! Return the intersection area of the given surface to the original one.
		The returned surface is not valid in case of no intersection.
	*/
	Surface intersect(const Surface& other);

	/*! Intersects the original surface with the given one.
		The result contains the intersection surfaces in first and the difference surfaces (original - other) in second.
	*/
	Surface::IntersectionResult intersect2(const Surface& other);

	bool merge(const Surface& subsurface);

	bool addSubSurface(const Surface& subsurface);

	std::vector<std::pair<size_t,size_t>> samePoints(const Surface& other) const;

	bool check();

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	double area() const;

	static MatchResult findFirstSurfaceMatchIndex(const std::vector<Surface>& wallSurfaces, const std::vector<Surface>& spaceSurfaces, double minDist);


	std::string								m_name;
	int										m_id;
	int										m_elementEntityId;
	int										m_openingId;
	bool									m_valid;
	PositionType							m_positionType;
	bool									m_virtualSurface;
	std::vector<SubSurface>					m_subSurfaces;

private:
	std::vector<IBKMK::Vector3D>			m_polyVect;
	std::vector<IBKMK::Vector2D>			m_polyVect2D;
	carve::geom::plane<3>					m_planeCarve;
	PlaneNormal								m_planeNormal;
};


struct Surface::IntersectionResult {

	bool isValid() const {
		if(m_intersections.empty())
			return false;
		for(const auto& poly : m_intersections)
			if(poly.polygon().size() > 2)
				return true;

		return false;
	}

	std::vector<Surface>				m_intersections;
	std::vector<Surface>				m_diffBaseMinusClip;
	std::vector<std::vector<Surface>>	m_holesBaseMinusClip;
	std::vector<Surface>				m_diffClipMinusBase;
	std::vector<std::vector<Surface>>	m_holesClipMinusBase;
};


} // namespace IFCC

#endif // IFCC_SURFACE_H
