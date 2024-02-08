#ifndef IFCC_GeometricHelperClassesH
#define IFCC_GeometricHelperClassesH

#include <carve/geom.hpp>

#include <array>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Vector2D.h>
#include <IBKMK_Polygon3D.h>

namespace IFCC {

class RotationMatrix;

/*! Class represents a plane in 3D given in Hesse normal form.*/
class PlaneHesseNormal
{
public:
	/*! Standard constructor. Create a non valid object.*/
	PlaneHesseNormal();

	/*! Constructor take data from carve plane.*/
	PlaneHesseNormal(const carve::geom::plane<3>& plane);

	/*! Constructor creates the plane from the given polygon. Polygon should lie in one plane.
		Create a non valid object if polygon is not valid or not in one plane.
	*/
	PlaneHesseNormal(const std::vector<IBKMK::Vector3D>& polygon);

	IBKMK::Vector3D	m_n0;	///< Normalized normal vector
	double			m_d;	///< Distance

private:
	/*! Resolve plane data from given polygon.
		Return false and create a non valid object in case of non valid polygon.*/
	bool getPlane(const std::vector<IBKMK::Vector3D>& polygon);
};

/*! Class represents a plane in 3D given in normal form.*/
class PlaneNormal
{
public:
	/*! Plane classification.*/
	enum PolygonPlane {
		Unknown,		///< Undetermined plane class
		SlopedPlane,	///< Plane is not planar to a axis plane
		XYPlane,		///< Plane is planar to xy axis plane
		XZPlane,		///< Plane is planar to xz axis plane
		YZPlane		///< Plane is planar to yz axis plane
	};

	/*! Standard constructor. Create a non valid object.*/
	PlaneNormal();

	/*! Standard destructor.*/
	~PlaneNormal();

	/*! Standard copy constructor.*/
	PlaneNormal(const PlaneNormal&);

	/*! Copy assignment operator.*/
	PlaneNormal& operator=(const PlaneNormal&);

	/*! Constructor create a object from given plane in Hesse normal form for the given position.
		\param plane Plane in Hesse normal form
		\param pos Position vector for new plane
	*/
	PlaneNormal(const PlaneHesseNormal& plane, const IBKMK::Vector3D& pos);

	/*! Constructor create a object from given plane in Hesse normal form. Position vector will be evaluated from given polygon.
		\param plane Plane in Hesse normal form
		\param polygon Polygon for position vector
	*/
	PlaneNormal(const PlaneHesseNormal& plane, const std::vector<IBKMK::Vector3D>& polygon);

	/*! Constructor create a object from given polygon.
		\param polygon Polygon for position vector and plane
	*/
	PlaneNormal(const std::vector<IBKMK::Vector3D>& polygon);

	/*! Convert given 3D point into plane coordinates. Point should lie in plane.*/
	IBKMK::Vector2D convert3DPoint(const IBKMK::Vector3D& point) const;

	/*! Convert point given in 2D plane coordinates back to global 3D coordinates.*/
	IBKMK::Vector3D convert3DPointInv(const IBKMK::Vector2D& point) const;

	bool					m_valid;				///< If true plane is valid otherwise not.
	PolygonPlane			m_polygonPlane;			///< Plane classification
	double					m_distance;				///< Distance of Hesse normal form
	IBKMK::Vector3D			m_pos;					///< Position vector
	IBKMK::Vector3D			m_rot_pos;				///< Rotated position vector
	IBKMK::Vector3D			m_lx;					///< X-axis
	IBKMK::Vector3D			m_ly;					///< Y-axis
	IBKMK::Vector3D			m_lz;					///< Z-axis

private:
	/*! Evaluates the plane classification.*/
	void setPlaneProperties(const PlaneHesseNormal& plane);

	std::unique_ptr<RotationMatrix>	m_rotationMatrix;		///< Rotation matrix for using in convert3DPoint
	std::unique_ptr<RotationMatrix>	m_rotationMatrixInv;	///< Inverse rotation matrix for using in convert3DPointInv.
};

} // namespace IFCC

#endif // IFCC_GeometricHelperClassesH
