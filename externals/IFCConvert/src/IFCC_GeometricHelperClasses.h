#ifndef IFCC_GeometricHelperClassesH
#define IFCC_GeometricHelperClassesH

#include <carve/geom.hpp>

#include <array>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Vector2D.h>
#include <IBKMK_Polygon3D.h>

namespace IFCC {

/*! Matrix for rotation of 3D geometry objects (3x3).*/
class RotationMatrix {
public:
	/*! Matrix data with different access possibilities.*/
	union {
		double m_m[3][3];	///< As matrix
		double m_v[9];		///< As vector
		/*! as single variables.*/
		struct {
			double m_11, m_12, m_13;
			double m_21, m_22, m_23;
			double m_31, m_32, m_33;
		};
	};

	/*! Constructor with single variables.*/
	RotationMatrix(double __11, double __21, double __31, double __12, double __22,
				   double __32, double __13, double __23, double __33);

	/*! Constructor with matrix data.*/
	RotationMatrix(double m[3][3]);

	/*! Constructor with plain vector.*/
	RotationMatrix(double v[9]);

	/*! Standard constructor. Create a identity matrix.*/
	RotationMatrix();

	/*! Return a LU factorized version.*/
	RotationMatrix lu() const;

	/*! Backsolving with given result vector of size 3.
		Can only be used with an LU factorized matrix.*/
	void backsolve(double * b) const;

	/*! Create inverse of rotation matrix.*/
	bool inverse(RotationMatrix& invMat) const;

	/*! Equality operator.*/
	friend bool operator==(const RotationMatrix& A, const RotationMatrix& B);

	/*! Non-equal operator.*/
	friend bool operator!=(const RotationMatrix& A, const RotationMatrix& B);

	/*! Product of rotation matrix with 3D vector.*/
	friend IBKMK::Vector3D operator*(const RotationMatrix& A, const IBKMK::Vector3D& b);

	/*! Product of 3D vector with rotation matrix.*/
	friend IBKMK::Vector3D& operator*=(IBKMK::Vector3D& b, const RotationMatrix& A);

	/*! Product of two matrices.*/
	friend RotationMatrix operator*(const RotationMatrix& A, const RotationMatrix& B);

private:
	/*! Helper function for determinant.*/
	double determinantOfMinor( int theRowHeightY, int theColumnWidthX ) const;

	/*! Determinant of matrix. Helper function for inverse.*/
	double determinant() const;
};

/*! Transformation matrix for 3D objects (4x4).*/
class TransformationMatrix {
public:
	/*! Matrix data with different access possibilities.*/
	union {
		double m_m[4][4];	///< as matrix
		double m_v[16];		///< as vector
		/*! as single variables.*/
		struct {
			// transposed
			double m_11, m_12, m_13, m_14;
			double m_21, m_22, m_23, m_24;
			double m_31, m_32, m_33, m_34;
			double m_41, m_42, m_43, m_44;
		};
	};

	/*! Constructor with single variables.*/
	TransformationMatrix(double __11, double __21, double __31, double __41,
						 double __12, double __22, double __32, double __42,
						 double __13, double __23, double __33, double __43,
						 double __14, double __24, double __34, double __44);

	/*! Constructor with matrix data.*/
	TransformationMatrix(double m[4][4]);

	/*! Constructor with plain vector.*/
	TransformationMatrix(double v[16]);

	/*! Standard constructor. Create a identity matrix.*/
	TransformationMatrix();

	/*! Return a LU factorized version.*/
	TransformationMatrix lu();

	/*! Backsolving with given result vector of size 3.
		Can only be used with an LU factorized matrix.*/
	void backsolve(double * b);

	/*! Matrix element cleaning. Set all elements near zero to zero and all elements near one to one.*/
	void clean();

	/*! Create inverse of transformation matrix.
		Return a deafult matrix in case of errors (singularities).
	*/
	TransformationMatrix inverse();

	/*! Equality operator.*/
	friend bool operator==(const TransformationMatrix& A, const TransformationMatrix& B);

	/*! Non-equal operator.*/
	friend bool operator!=(const TransformationMatrix& A, const TransformationMatrix& B);

	/*! Return product of matrix with a 3D vector.*/
	friend IBKMK::Vector3D operator*(const TransformationMatrix& A, const IBKMK::Vector3D& b);

	/*! Product of 3D vector with transformation matrix.*/
	friend IBKMK::Vector3D& operator*=(IBKMK::Vector3D& b, const TransformationMatrix& A);

	/*! Product of two matrices.*/
	friend TransformationMatrix operator*(const TransformationMatrix& A, const TransformationMatrix& B);

	/*! Return transformation matrix rotated by given angle around given axis.*/
	friend TransformationMatrix ROT(double angle, const IBKMK::Vector3D& axis);

	/*! Return transformation matrix rotated by given angle around axis given by coordinates.*/
	friend TransformationMatrix ROT(double angle, double x, double y, double z);

	/*! Return transformation matrix translated by given coordinates.*/
	friend TransformationMatrix TRANS(double x, double y, double z);

	/*! Return transformation matrix translated by given vector.*/
	friend TransformationMatrix TRANS(const IBKMK::Vector3D& v);

	/*! Return transformation matrix scaled by given coordinates.*/
	friend TransformationMatrix SCALE(double x, double y, double z);

	/*! Return transformation matrix scaled by given vector.*/
	friend TransformationMatrix SCALE(const IBKMK::Vector3D& v);
};

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

	RotationMatrix			m_rotationMatrix;		///< Rotation matrix for using in convert3DPoint
	RotationMatrix			m_rotationMatrixInv;	///< Inverse rotation matrix for using in convert3DPointInv.

private:
	/*! Evaluates the plane classification.*/
	void setPlaneProperties(const PlaneHesseNormal& plane);
};

} // namespace IFCC

#endif // IFCC_GeometricHelperClassesH
