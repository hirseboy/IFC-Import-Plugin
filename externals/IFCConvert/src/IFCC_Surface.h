#ifndef IFCC_SurfaceH
#define IFCC_SurfaceH

#include <carve/mesh.hpp>

#include <tinyxml.h>

//#include <VICUS_Surface.h>

#include <ifcpp/IFC4/include/IfcInternalOrExternalEnum.h>

#include "IFCC_Types.h"
#include "IFCC_Helper.h"
#include "IFCC_GeometricHelperClasses.h"
#include "IFCC_SubSurface.h"

namespace IFCC {

/*! Classe represents a surface which can be represented as splanar polygon.*/
class Surface
{
public:
	/*! Position of the surface relted to a building.*/
	enum PositionType {
		PT_Internal,
		PT_External,
		PT_External_Ground,
		PT_Unknown
	};

	struct IntersectionResult;

	/*! Default constructor. Create a non valid object*/
	Surface();

	/*! Constructor create a surface from a carve Face object.
		\param face Face object in 3D from carve lib.
	*/
	Surface(carve::mesh::Face<3>* face);

	/*! Create a surface from a 3D polygon.
		\param polygon Single 3D polygon. It should be planar.
	*/
	Surface(const polygon3D_t& polygon);

	void set(int id, int elementId, const std::string& name, bool isVirtual);

	/*! Return the distance between two parallel surfaces in the unit of the coordinates.
		It return the maximum double number if the two surfaces are not parallel.
		\param other Other surface for checking.
	*/
	double distanceToParallelPlane(const Surface& other) const;

	/*! Return true if both surfaces are parallel.*/
	bool isParallelTo(const Surface& other) const;

	/*! Return the internal polygon.*/
	const std::vector<IBKMK::Vector3D>& polygon() const {
		return m_polyVect;
	}

	/*! Return true if the given surface intersects the original one.
		\param other Other surface for checking.
	*/
	bool isIntersected(const Surface& other) const;

	/*! Return the intersection area of the given surface to the original one.
		The returned surface is not valid in case of no intersection.
		\param other Other surface for intersecting.
	*/
	Surface intersect(const Surface& other) const;

	/*! Intersects the original surface with the given one.
		The result contains the intersection surfaces and the resulting difference surfaces ('org - other' and 'other - org').
		For more details \sa Surface::IntersectionResult.
		\param other Other surface for calculations.
		\return Object of type Surface::IntersectionResult contains all calculated surfaces and holes (if exist) of first level.
	*/
	Surface::IntersectionResult intersect2(const Surface& other) const;

	/*! Return the rest (difference) area of the original surface minus the given one.
		The result vector is empty in case of no intersection.
		\param other Vector of difference surfaces
	*/
	std::vector<Surface> difference(const Surface& other) const;

	/*! Merge the given subsurface into the current surface.
		If the original surface was intersected by this subsurface the resulting surface is complete without holes.
		The merging is performed in 2D level using clipper library (\sa mergePolygons).
		\return True if the subsurface had an intersection.
	*/
	bool merge(const Surface& subsurface);

	/*! Add the given surface as subsurface to the internal subsurface list.
		While this process the subsurface will be converted into SubSurface type which contains only a 2D polygon in the plane of the current surface.
		\param subsurface Surface which should be a subsurface of the current one
		\return If false the resulting subsurface is not valid and nothing will be added
	*/
	bool addSubSurface(const Surface& subsurface);

	/*! Check if current and given polygon contains similar points. If found it collects the indices of these points and return this vector.
		The function checks all points independent of the order.
		\param other Other surface for checking.
		\return vector of indices of similar points. First is the current polygon and second the other one.
	*/
	std::vector<std::pair<size_t,size_t>> samePoints(const Surface& other) const;

	/*! Return the area of the surface.*/
	double area() const;

	/*! Flip the surfrace polygone.*/
	void flip();

	/*! Write the surface in vicus xml format including all subsurfaces.*/
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Set the position type of the surface.
		\param type Position type from IFC space boundary object.
	*/
	void setSurfaceType(IfcInternalOrExternalEnum::IfcInternalOrExternalEnumEnum type);

	std::vector<SubSurface> subSurfaces() const {
		return m_subSurfaces;
	}

	/*! Return the object id.*/
	int id() const {
		return m_id;
	}

	/*! Return the id of the corresponding building element.*/
	int elementId() const {
		return m_elementEntityId;
	}

	PositionType positionType() const {
		return m_positionType;
	}

	/*! Return if the object is valid.*/
	bool isValid() const;

//	/*! Create a VICUS surface object and return this.
//		The returned object contains all transferable data.
//	*/
//	VICUS::Surface getVicusObject(std::map<int,int>& idMap, int& nextid) const;

	/*! Return name of the object.*/
	std::string name() const {
		return m_name;
	}

	bool isVirtual() const  {
		return m_virtualSurface;
	}

	bool isMissing() const {
		return m_elementEntityId == -1;
	}


private:
	std::string								m_name;
	int										m_id;
	int										m_elementEntityId;
	bool									m_virtualSurface;
	PositionType							m_positionType;
	std::vector<IBKMK::Vector3D>			m_polyVect;
	std::vector<SubSurface>					m_subSurfaces;
	carve::geom::plane<3>					m_planeCarve;
	PlaneNormal								m_planeNormal;
};


/*! Contains results of function intersectPolygons2.
*/
struct Surface::IntersectionResult {

	/*! Result is only valid if at least one intersection exist and contains a valid polygon.*/
	bool isValid() const {
		if(m_intersections.empty())
			return false;
		for(const auto& poly : m_intersections)
			if(poly.polygon().size() > 2)
				return true;

		return false;
	}

	/*! Vector of intersection surfaces.*/
	std::vector<Surface>				m_intersections;
	/*! Vector of surfaces from operation 'BasePolygon - ClipPolygon'*/
	std::vector<Surface>				m_diffBaseMinusClip;
	/*! Vector of hole surfaces for each existing diffBaseMinusClip-polygon.
		First dimension must be the same as m_diffBaseMinusClip.
	*/
	std::vector<std::vector<Surface>>	m_holesBaseMinusClip;
	/*! Vector of surfaces from operation 'ClipPolygon - BasePolygon'*/
	std::vector<Surface>				m_diffClipMinusBase;
	/*! Vector of hole surfaces for each existing diffClipMinusBase-polygon.
		First dimension must be the same as m_diffClipMinusBase.
	*/
	std::vector<std::vector<Surface>>	m_holesClipMinusBase;
};

void surfacesFromRepresentation(std::shared_ptr<ProductShapeData> productShape, std::vector<Surface>& surfaces);

} // namespace IFCC

#endif // IFCC_SURFACE_H
