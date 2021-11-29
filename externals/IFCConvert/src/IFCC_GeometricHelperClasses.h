#ifndef IFCC_GeometricHelperClassesH
#define IFCC_GeometricHelperClassesH

#include <carve/geom.hpp>

#include <array>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Vector2D.h>
#include <IBKMK_Polygon3D.h>

namespace IFCC {

class RotationMatrix {
public:
	union {
		double m_m[3][3];
		double m_v[9];
		struct {
			// transposed
			double m_11, m_12, m_13;
			double m_21, m_22, m_23;
			double m_31, m_32, m_33;
		};
	};

	RotationMatrix(double __11, double __21, double __31, double __12, double __22,
				   double __32, double __13, double __23, double __33);

	RotationMatrix(double m[3][3]);

	RotationMatrix(double v[9]);

	RotationMatrix();

	RotationMatrix lu() const;

	/*! Can only be used with an LU factorized matrix.*/
	void backsolve(double * b) const;

	bool inverse(RotationMatrix& invMat) const;

	friend bool operator==(const RotationMatrix& A, const RotationMatrix& B);

	friend bool operator!=(const RotationMatrix& A, const RotationMatrix& B);

	friend IBKMK::Vector3D operator*(const RotationMatrix& A, const IBKMK::Vector3D& b);

	friend IBKMK::Vector3D& operator*=(IBKMK::Vector3D& b, const RotationMatrix& A);

	friend RotationMatrix operator*(const RotationMatrix& A, const RotationMatrix& B);

private:
	double determinantOfMinor( int theRowHeightY, int theColumnWidthX ) const;

	double determinant() const;
};


class TransformationMatrix {
public:
	// access: .m[col][row], .v[col * 4 + row], ._cr
	union {
		double m_m[4][4];
		double m_v[16];
		struct {
			// transposed
			double m_11, m_12, m_13, m_14;
			double m_21, m_22, m_23, m_24;
			double m_31, m_32, m_33, m_34;
			double m_41, m_42, m_43, m_44;
		};
	};
	TransformationMatrix(double __11, double __21, double __31, double __41,
						 double __12, double __22, double __32, double __42,
						 double __13, double __23, double __33, double __43,
						 double __14, double __24, double __34, double __44);

	TransformationMatrix(double _m[4][4]);

	TransformationMatrix(double _v[16]);

	TransformationMatrix();

	TransformationMatrix lu();

	/*! Can only be used with an LU factorized matrix.*/
	void backsolve(double * b);

	void clean();

	TransformationMatrix inverse();

	friend bool operator==(const TransformationMatrix& A, const TransformationMatrix& B);

	friend bool operator!=(const TransformationMatrix& A, const TransformationMatrix& B);

	friend IBKMK::Vector3D operator*(const TransformationMatrix& A, const IBKMK::Vector3D& b);

	friend IBKMK::Vector3D& operator*=(IBKMK::Vector3D& b, const TransformationMatrix& A);

	friend TransformationMatrix operator*(const TransformationMatrix& A, const TransformationMatrix& B);

	friend TransformationMatrix ROT(double angle, const IBKMK::Vector3D& axis);

	friend TransformationMatrix ROT(double angle, double x, double y, double z);

	friend TransformationMatrix TRANS(double x, double y, double z);

	friend TransformationMatrix TRANS(const IBKMK::Vector3D& v);

	friend TransformationMatrix SCALE(double x, double y, double z);

	friend TransformationMatrix SCALE(const IBKMK::Vector3D& v);
};


class PlaneHesseNormal
{
public:
	PlaneHesseNormal();

	PlaneHesseNormal(const carve::geom::plane<3>& plane);

	PlaneHesseNormal(const std::vector<IBKMK::Vector3D>& polygon);

	IBKMK::Vector3D	m_n0;
	double			m_d;

private:
	bool getPlane(const std::vector<IBKMK::Vector3D>& polygon);
};

class PlaneNormal
{
public:
	enum PolygonPlane {
		Unknown,
		SlopedPlane,
		XYPlane,
		XZPlane,
		YZPlane
	};

	PlaneNormal();

	PlaneNormal(const PlaneHesseNormal& plane, const IBKMK::Vector3D& pos);

	PlaneNormal(const PlaneHesseNormal& plane, const std::vector<IBKMK::Vector3D>& polygon);

	PlaneNormal(const std::vector<IBKMK::Vector3D>& polygon);

	IBKMK::Vector2D convert3DPoint(const IBKMK::Vector3D& point) const;

	IBKMK::Vector3D convert3DPointInv(const IBKMK::Vector2D& point) const;

	static PlaneNormal createXYPlane();

	bool					m_valid;
	PolygonPlane			m_polygonPlane;
	double					m_distance;
	IBKMK::Vector3D			m_pos;
	IBKMK::Vector3D			m_rot_pos;
	IBKMK::Vector3D			m_lx;
	IBKMK::Vector3D			m_ly;
	IBKMK::Vector3D			m_lz;

	TransformationMatrix	m_transMatrix;
	TransformationMatrix	m_transMatrixInv;

	RotationMatrix			m_rotationMatrix;
	RotationMatrix			m_rotationMatrixInv;

private:
	void setPlaneProperties(const PlaneHesseNormal& plane);
};

} // namespace IFCC

#endif // IFCC_GEOMETRICHELPERCLASSES_H
