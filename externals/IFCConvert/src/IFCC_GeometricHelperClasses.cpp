#include "IFCC_GeometricHelperClasses.h"

#include <IBK_math.h>
#include <IBK_assert.h>

#include <fstream>


namespace IFCC {


static bool rotateVector(const IBKMK::Vector3D& vect, PlaneNormal::PolygonPlane plane, IBKMK::Vector2D& res) {
	switch(plane) {
		case PlaneNormal::XYPlane: {
			res.m_x = vect.m_x;
			res.m_y = vect.m_y;
			return true;
		}
		case PlaneNormal::XZPlane: {
			res.m_x = vect.m_x;
			res.m_y = vect.m_z;
			return true;
		}
		case PlaneNormal::YZPlane: {
			res.m_x = vect.m_y;
			res.m_y = vect.m_z;
			return true;
		}
		case PlaneNormal::SlopedPlane: {
			return false;
		}
		case PlaneNormal::Unknown: {
			return false;
		}
	}
	return false;
}

static auto vectorDirection(IBKMK::Vector3D vect) -> double {
	vect.normalize();
	int zeros = 0;
	int neg = 0;
	int pos = 0;
	if(IBK::near_zero(vect.m_x))
		++zeros;
	else if(vect.m_x > 0)
		++pos;
	else ++neg;
	if(IBK::near_zero(vect.m_y))
		++zeros;
	else if(vect.m_y > 0)
		++pos;
	else ++neg;
	if(IBK::near_zero(vect.m_z))
		++zeros;
	else if(vect.m_z > 0)
		++pos;
	else ++neg;
	if(pos > neg) return 1.0;
	if(zeros == 3) return 0.0;
	return -1.0;
}

static bool rotateVectorBack(const IBKMK::Vector2D& vect, PlaneNormal::PolygonPlane plane, double dist, IBKMK::Vector3D& res) {
	switch(plane) {
		case PlaneNormal::XYPlane: {
			res.m_x = vect.m_x;
			res.m_y = vect.m_y;
			res.m_z = dist;
			return true;
		}
		case PlaneNormal::XZPlane: {
			res.m_x = vect.m_x;
			res.m_y = dist;
			res.m_z = vect.m_y;
			return true;
		}
		case PlaneNormal::YZPlane: {
			res.m_x = dist;
			res.m_y = vect.m_x;
			res.m_z = vect.m_y;
			return true;
		}
		case PlaneNormal::SlopedPlane: {
			return false;
		}
		case PlaneNormal::Unknown: {
			return false;
		}
	}
	return false;
}

// quaternion helper
struct Quaternion {
  double m_x, m_y, m_z, m_w;

  Quaternion(double x, double y, double z, double w)
	  :
		m_x(x),
		m_y(y),
		m_z(z),
		m_w(w)
  {}

  Quaternion(double angle, const IBKMK::Vector3D& axis) {
	double s = axis.magnitude();
	if (!IBK::near_zero(s)) {
	  double c = 1.0 / s;
	  double omega = -0.5 * angle;
	  s = sin(omega);
	  m_x = axis.m_x * c * s;
	  m_y = axis.m_y * c * s;
	  m_z = axis.m_z * c * s;
	  m_w = std::cos(omega);
	  normalize();
	}
	else {
	  m_x = m_y = m_z = 0.0;
	  m_w = 1.0;
	}
  }

  double lengthSquared() const {
		return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
  }

  double length() const {
	  return std::sqrt(lengthSquared());
  }

  Quaternion normalized() const {
	  return Quaternion(*this).normalize();
  }

  Quaternion& normalize() {
	double l = length();
	if (l == 0.0) {
	  m_x = 1.0;
	  m_y = 0.0;
	  m_z = 0.0;
	  m_w = 0.0;
	}
	else {
	  m_x /= l;
	  m_y /= l;
	  m_z /= l;
	  m_w /= l;
	}
	return *this;
  }
};

// Rotation matrix 3x3

RotationMatrix::RotationMatrix(double __11, double __21, double __31, double __12, double __22,
		double __32, double __13, double __23, double __33)  :
	m_11(__11),
	m_12(__12),
	m_13(__13),
	m_21(__21),
	m_22(__22),
	m_23(__23),
	m_31(__31),
	m_32(__32),
	m_33(__33)
{}

RotationMatrix::RotationMatrix(double m[3][3]) {
	std::memcpy(m_m, m, sizeof(m_m));
}

RotationMatrix::RotationMatrix(double v[9]) {
	std::memcpy(m_v, v, sizeof(m_v));
}

RotationMatrix::RotationMatrix() :
  m_11(1.00),
  m_12(0.00),
  m_13(0.00),
  m_21(0.00),
  m_22(1.00),
  m_23(0.00),
  m_31(0.00),
  m_32(0.00),
  m_33(1.00)
{}

RotationMatrix RotationMatrix::lu() const {
	RotationMatrix res(*this);

	if (res.m_v[0] == 0)
		return RotationMatrix();

	/* column 0 */
	res.m_v[1] /= res.m_v[0];
	res.m_v[2] /= res.m_v[0];

	/* column 1 */
	res.m_v[4] -= res.m_v[1]*res.m_v[3];
	if (res.m_v[4] == 0)
		return RotationMatrix();

	res.m_v[5] -= res.m_v[2]*res.m_v[3];
	res.m_v[5] /= res.m_v[4];

	/* column 2 */
	res.m_v[7] -= res.m_v[1]*res.m_v[6];
	res.m_v[8] -= res.m_v[2]*res.m_v[6] + res.m_v[5]*res.m_v[7];
	if (res.m_v[8] == 0)
		return RotationMatrix();

	return res;
}

/*! Can only be used with an LU factorized matrix.*/
void RotationMatrix::backsolve(double * b) const {
	b[1] -= m_v[1]*b[0];
	b[2] -= m_v[2]*b[0] + m_v[5]*b[1];

	/* backward elimination */
	b[2] /= m_v[8];
	b[1] -= m_v[7]*b[2];
	b[1] /= m_v[4];
	b[0] -= m_v[6]*b[2] + m_v[3]*b[1];
	b[0] /= m_v[0];
}

double RotationMatrix::determinantOfMinor( int theRowHeightY, int theColumnWidthX ) const {
	int x1 = theColumnWidthX == 0 ? 1 : 0;  /* always either 0 or 1 */
	int x2 = theColumnWidthX == 2 ? 1 : 2;  /* always either 1 or 2 */
	int y1 = theRowHeightY   == 0 ? 1 : 0;  /* always either 0 or 1 */
	int y2 = theRowHeightY   == 2 ? 1 : 2;  /* always either 1 or 2 */

	return ( m_m[y1][x1]  * m_m[y2][x2] )
			-  ( m_m[y1][x2]  *  m_m[y2][x1] );
}

double RotationMatrix::determinant() const {
	return ( m_m[0][0]      *  determinantOfMinor( 0, 0 ) )
			-  ( m_m[0][1]  *  determinantOfMinor( 0, 1 ) )
			+  ( m_m[0][2]  *  determinantOfMinor( 0, 2 ) );
}

bool RotationMatrix::inverse(RotationMatrix& invMat) const {
	double det = determinant();

	/* Arbitrary for now.  This should be something nicer... */
	if ( std::fabs(det) < 1e-2 )   {
		invMat = RotationMatrix();
		return false;
	}

	double oneOverDeterminant = 1.0 / det;

	for (size_t y = 0;  y < 3;  y ++ )
		for (size_t x = 0;  x < 3;  x ++   ) {
			/* Rule is inverse = 1/det * minor of the TRANSPOSE matrix.  *
		 * Note (y,x) becomes (x,y) INTENTIONALLY here!              */
			invMat.m_m[y][x]
					= determinantOfMinor( x, y ) * oneOverDeterminant;

			/* (y0,x1)  (y1,x0)  (y1,x2)  and (y2,x1)  all need to be negated. */
			if( 1 == ((x + y) % 2) )
				invMat.m_m[y][x] = - invMat.m_m[y][x];
		}

	return true;
}

bool operator==(const RotationMatrix& A, const RotationMatrix& B) {
	for (size_t i = 0; i < 9; ++i) {
		if (!IBK::near_equal(A.m_v[i], B.m_v[i])) {
			return false;
		}
	}
	return true;
}

bool operator!=(const RotationMatrix& A, const RotationMatrix& B) {
	return !(A == B);
}

IBKMK::Vector3D operator*(const RotationMatrix& A, const IBKMK::Vector3D& b) {
	return IBKMK::Vector3D(A.m_11 * b.m_x + A.m_21 * b.m_y + A.m_31 * b.m_z,
						   A.m_12 * b.m_x + A.m_22 * b.m_y + A.m_32 * b.m_z,
						   A.m_13 * b.m_x + A.m_23 * b.m_y + A.m_33 * b.m_z);
}

IBKMK::Vector3D& operator*=(IBKMK::Vector3D& b, const RotationMatrix& A) {
	b = A * b;
	return b;
}

RotationMatrix operator*(const RotationMatrix& A, const RotationMatrix& B) {
	RotationMatrix c;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			c.m_m[i][j] = 0.0;
			for (int k = 0; k < 3; k++) {
				c.m_m[i][j] += A.m_m[k][j] * B.m_m[i][k];
			}
		}
	}
	return c;
}

// Transformation matrix
TransformationMatrix::TransformationMatrix(double __11, double __21, double __31, double __41,
					 double __12, double __22, double __32, double __42,
					 double __13, double __23, double __33, double __43,
					 double __14, double __24, double __34, double __44) :
	m_11(__11),
	m_12(__12),
	m_13(__13),
	m_14(__14),
	m_21(__21),
	m_22(__22),
	m_23(__23),
	m_24(__24),
	m_31(__31),
	m_32(__32),
	m_33(__33),
	m_34(__34),
	m_41(__41),
	m_42(__42),
	m_43(__43),
	m_44(__44)
{
}

TransformationMatrix::TransformationMatrix(double _m[4][4]) {
	std::memcpy(m_m, _m, sizeof(m_m));
}

TransformationMatrix::TransformationMatrix(double _v[16]) {
	std::memcpy(m_v, _v, sizeof(m_v));
}

TransformationMatrix::TransformationMatrix() :
	m_11(1),
	m_12(0),
	m_13(0),
	m_14(0),
	m_21(0),
	m_22(1),
	m_23(0),
	m_24(0),
	m_31(0),
	m_32(0),
	m_33(1),
	m_34(0),
	m_41(0),
	m_42(0),
	m_43(0),
	m_44(1)
{
}

void TransformationMatrix::clean() {
	for(size_t i=0; i<16; ++i) {
		if(IBK::near_zero(m_v[i]))
			m_v[i] = 0;
		if(IBK::near_equal(m_v[i],1.0))
			m_v[i] = 1.0;
	}
}


TransformationMatrix TransformationMatrix::lu() {
	TransformationMatrix res(*this);

	if (res.m_v[0] == 0)
			return TransformationMatrix();

	/* column 0 */
	res.m_v[1] /= res.m_v[0];
	res.m_v[2] /= res.m_v[0];
	res.m_v[3] /= res.m_v[0];

	/* column 1 */
	res.m_v[5] -= res.m_v[1]*res.m_v[4];
	if (res.m_v[5] == 0)
		return TransformationMatrix();

	res.m_v[6] -= res.m_v[2]*res.m_v[4];
	res.m_v[6] /= res.m_v[5];
	res.m_v[7] -= res.m_v[3]*res.m_v[4];
	res.m_v[7] /= res.m_v[5];

	/* column 2 */
	res.m_v[9] -= res.m_v[1]*res.m_v[8];
	res.m_v[10] -= res.m_v[2]*res.m_v[8] + res.m_v[6]*res.m_v[9];
	if (res.m_v[10] == 0)
		return TransformationMatrix();

	res.m_v[11] -= res.m_v[3]*res.m_v[8] + res.m_v[7]*res.m_v[9];
	res.m_v[11] /= res.m_v[10];

	/* column 3 */
	res.m_v[13] -= res.m_v[1]*res.m_v[12];
	res.m_v[14] -= res.m_v[2]*res.m_v[12] + res.m_v[6]*res.m_v[13];
	res.m_v[15] -= res.m_v[3]*res.m_v[12] + res.m_v[7]*res.m_v[13] + res.m_v[11]*res.m_v[14];
	if (res.m_v[15] == 0)
		return TransformationMatrix();

	return res;
}

void TransformationMatrix::backsolve(double * b) {

	/* forward elimination */
	b[1] -= m_v[1]*b[0];
	b[2] -= m_v[2]*b[0] + m_v[6]*b[1];
	b[3] -= m_v[3]*b[0] + m_v[7]*b[1] + m_v[11]*b[2];

	/* backward elimination */
	b[3] /= m_v[15];
	b[2] -= m_v[14]*b[3];
	b[2] /= m_v[10];
	b[1] -= m_v[13]*b[3] + m_v[9]*b[2];
	b[1] /= m_v[5];
	b[0] -= m_v[12]*b[3] + m_v[8]*b[2] + m_v[4]*b[1];
	b[0] /= m_v[0];
}

TransformationMatrix TransformationMatrix::inverse() {
	TransformationMatrix alu = lu();
	TransformationMatrix B; // create ident matrix
	if(alu == B)
		return B;

	alu.backsolve(B.m_m[0]);
	alu.backsolve(B.m_m[1]);
	alu.backsolve(B.m_m[2]);
	alu.backsolve(B.m_m[3]);

	return B;
}

bool operator==(const TransformationMatrix& A, const TransformationMatrix& B) {
	for (size_t i = 0; i < 16; ++i) {
		if (!IBK::near_equal(A.m_v[i], B.m_v[i])) {
			return false;
		}
	}
	return true;
}

bool operator!=(const TransformationMatrix& A, const TransformationMatrix& B) {
	return !(A == B);
}

IBKMK::Vector3D operator*(const TransformationMatrix& A, const IBKMK::Vector3D& b) {
	return IBKMK::Vector3D(A.m_11 * b.m_x + A.m_21 * b.m_y + A.m_31 * b.m_z + A.m_41,
						   A.m_12 * b.m_x + A.m_22 * b.m_y + A.m_32 * b.m_z + A.m_42,
						   A.m_13 * b.m_x + A.m_23 * b.m_y + A.m_33 * b.m_z + A.m_43);
}

IBKMK::Vector3D& operator*=(IBKMK::Vector3D& b, const TransformationMatrix& A) {
	b = A * b;
	return b;
}

TransformationMatrix operator*(const TransformationMatrix& A, const TransformationMatrix& B) {
  TransformationMatrix c;
  for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 4; j++) {
	  c.m_m[i][j] = 0.0;
	  for (int k = 0; k < 4; k++) {
		c.m_m[i][j] += A.m_m[k][j] * B.m_m[i][k];
	  }
	}
  }
  return c;
}

static TransformationMatrix ROT(const Quaternion& q) {
  const double w = q.m_w;
  const double x = q.m_x;
  const double y = q.m_y;
  const double z = q.m_z;
  return TransformationMatrix(1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w,
				2 * x * z + 2 * y * w, 0.0, 2 * x * y + 2 * z * w,
				1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w, 0.0,
				2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w,
				1 - 2 * x * x - 2 * y * y, 0.0, 0.0, 0.0, 0.0, 1.0);
}

TransformationMatrix ROT(double angle, const IBKMK::Vector3D& axis) {
  return ROT(Quaternion(angle, axis));
}

TransformationMatrix ROT(double angle, double x, double y, double z) {
  return ROT(Quaternion(angle, IBKMK::Vector3D(x, y, z)));
}

TransformationMatrix TRANS(double x, double y, double z) {
  return TransformationMatrix(1.0, 0.0, 0.0, x, 0.0, 1.0, 0.0, y, 0.0, 0.0, 1.0, z, 0.0,
				0.0, 0.0, 1.0);
}

TransformationMatrix TRANS(const IBKMK::Vector3D& v) {
  return TRANS(v.m_x, v.m_y, v.m_z);
}

TransformationMatrix SCALE(double x, double y, double z) {
  return TransformationMatrix(x, 0.0, 0.0, 0.0, 0.0, y, 0.0, 0.0, 0.0, 0.0, z, 0.0, 0.0,
				0.0, 0.0, 1.0);
}

TransformationMatrix SCALE(const IBKMK::Vector3D& v) {
  return SCALE(v.m_x, v.m_y, v.m_z);
}


PlaneHesseNormal::PlaneHesseNormal() :
	m_d(0)
{

}

PlaneHesseNormal::PlaneHesseNormal(const carve::geom::plane<3>& plane) :
	m_n0(plane.N.x,plane.N.y,plane.N.z),
	m_d(plane.d)
{
};

PlaneHesseNormal::PlaneHesseNormal(const std::vector<IBKMK::Vector3D>& polygon) {
	getPlane(polygon);
}


static bool smallerVectZero(const IBKMK::Vector3D& vect) {
	if (vect.m_x < 0) {
		return true;
	}
	if (vect.m_x > 0) {
		return false;
	}
	if (vect.m_y < 0) {
		return true;
	}
	if (vect.m_y > 0) {
		return false;
	}
	if (vect.m_z < 0) {
		return true;
	}
	if (vect.m_z > 0) {
		return false;
	}
	return false;
}

bool PlaneHesseNormal::getPlane(const std::vector<IBKMK::Vector3D>& polygon) {

	if (polygon.size() < 3) {
		return false;
	}

	IBKMK::Vector3D C;
	IBKMK::Vector3D n; //move to here //vc++ optimize bug
	for(const IBKMK::Vector3D& vert : polygon) {
		C += vert;
	}
	C = C * (1.0/polygon.size());


	if (polygon.size() == 3) {
		n = (polygon[1] - polygon[0]).crossProduct(polygon[2] - polygon[0]);
	}
	else {
		size_t N = polygon.size();

		n = (polygon[N - 1] - C).crossProduct(polygon[0] - C);
		if (smallerVectZero(n)) {
			n = n * -1;
		}
		for (size_t i = 1; i < polygon.size(); ++i) {
			IBKMK::Vector3D v = (polygon[i] - C).crossProduct(polygon[i - 1] - C);
			if (smallerVectZero(v)) {
				v = v * -1.0;
			}
			n += v;
		}
	}

	double l = n.magnitude();

	if (l == 0.0) {
		n.m_x = 1.0;
		n.m_y = 0.0;
		n.m_z = 0.0;
	}
	else {
		n.normalize();
	}

	m_n0 = n;
	m_d = n.scalarProduct(C) * -1.0;
	return true;
}


PlaneNormal::PlaneNormal() :
	m_valid(false),
	m_polygonPlane(Unknown),
	m_distance(0)
{
}

PlaneNormal::PlaneNormal(const PlaneHesseNormal& plane, const IBKMK::Vector3D& pos) :
	m_valid(true),
	m_polygonPlane(Unknown),
	m_distance(0),
	m_pos(pos),
	m_rot_pos(pos)
{
	setPlaneProperties(plane);

	IBKMK::Vector3D A(1,1,1);
	if(IBK::near_zero(plane.m_n0.m_x)) {
		if(IBK::near_zero(plane.m_n0.m_y)) {
			// only z
			IBK_ASSERT(!IBK::near_zero(plane.m_n0.m_z));
			A.m_z = m_distance / plane.m_n0.m_z;
		}
		// z and y
		else {
			A.m_y = (m_distance - plane.m_n0.m_z) / plane.m_n0.m_y;
		}
	}
	else if(IBK::near_zero(plane.m_n0.m_y)) {
		// only x
		if(IBK::near_zero(plane.m_n0.m_z)) {
			A.m_x = m_distance / plane.m_n0.m_x;
		}
		// x and z
		else {
			A.m_x = (m_distance - plane.m_n0.m_y) / plane.m_n0.m_x;
		}
	}
	// x and y
	else {
		A.m_x = (m_distance - plane.m_n0.m_y - plane.m_n0.m_z) / plane.m_n0.m_x;
	}
	m_lx = A - pos;
	m_ly = m_lz.crossProduct(m_lx);

	m_lx.normalize();
	m_lz.normalize();
	m_ly.normalize();

//	double x = m_lx.m_x * m_pos.m_x + m_ly.m_x * m_pos.m_y + m_lz.m_x * m_pos.m_z ;
//	double y = m_lx.m_y * m_pos.m_x + m_ly.m_y * m_pos.m_y + m_lz.m_y * m_pos.m_z ;
//	double z = m_lx.m_z * m_pos.m_x + m_ly.m_z * m_pos.m_y + m_lz.m_z * m_pos.m_z;

	m_rotationMatrix = RotationMatrix(m_lx.m_x, m_ly.m_x, m_lz.m_x,
								  m_lx.m_y, m_ly.m_y, m_lz.m_y,
								  m_lx.m_z, m_ly.m_z, m_lz.m_z);

	bool inverseValid = m_rotationMatrix.inverse(m_rotationMatrixInv);
	if(!inverseValid)
		m_valid = false;

	m_rot_pos = m_rotationMatrixInv * m_pos * -1;

	m_transMatrix = TransformationMatrix(m_lx.m_x, m_ly.m_x, m_lz.m_x, m_rot_pos.m_x,
								  m_lx.m_y, m_ly.m_y, m_lz.m_y, m_rot_pos.m_y,
								  m_lx.m_z, m_ly.m_z, m_lz.m_z, m_rot_pos.m_z,
								  0, 0, 0, 1);

	m_transMatrixInv = m_transMatrix.inverse();

//	matrix.clean();
//	matrixInv.clean();
}

PlaneNormal::PlaneNormal(const PlaneHesseNormal& plane, const std::vector<IBKMK::Vector3D>& polygon) :
	m_valid(true),
	m_polygonPlane(Unknown),
	m_distance(0)
{
	IBK_ASSERT(polygon.size() > 2);

	setPlaneProperties(plane);
	IBK_ASSERT(m_polygonPlane != Unknown);

	if(m_polygonPlane == SlopedPlane) {
		IBKMK::Vector3D p1;
		IBKMK::Vector3D p2;
		IBKMK::Vector3D startVect;
		bool found = false;
		for(size_t i=0; i<polygon.size()-1; ++i) {
			p1 = polygon[i];
			p2 = polygon[i+1];
			startVect = p2 - p1;
			startVect.normalize();
			if(!IBK::near_zero(startVect.m_x) || !IBK::near_zero(startVect.m_y)) {
				found = true;
				break;
			}
		}
		m_valid = found;
		m_pos = p1;

		m_lx = p2 - p1;
		m_lx.normalize();
		if(IBK::near_zero(m_lx.m_x)) {
			m_ly = p2 - p1;
			m_lx = m_lz.crossProduct(m_ly);
		}
		else {
			m_lx = p2 - p1;
			m_ly = m_lz.crossProduct(m_lx);
		}

		m_lx.normalize();
		m_ly.normalize();
		m_lz.normalize();
	}
	else if(m_polygonPlane == XYPlane) {
		m_lx = IBKMK::Vector3D(1,0,0);
		m_ly = IBKMK::Vector3D(0,1,0);
		m_lz = IBKMK::Vector3D(0,0,1);
	}
	else if(m_polygonPlane == XZPlane) {
		m_lx = IBKMK::Vector3D(1,0,0);
		m_ly = IBKMK::Vector3D(0,0,1);
		m_lz = IBKMK::Vector3D(0,-1,0);
	}
	else if(m_polygonPlane == YZPlane) {
		m_lx = IBKMK::Vector3D(0,0,1);
		m_ly = IBKMK::Vector3D(0,1,0);
		m_lz = IBKMK::Vector3D(-1,0,0);
	}
	else {
		// should never happen
	}

	m_rotationMatrix = RotationMatrix(m_lx.m_x, m_ly.m_x, m_lz.m_x,
									  m_lx.m_y, m_ly.m_y, m_lz.m_y,
									  m_lx.m_z, m_ly.m_z, m_lz.m_z);

	bool inverseValid = m_rotationMatrix.inverse(m_rotationMatrixInv);
	if(!inverseValid)
		m_valid = false;

	m_rot_pos = m_rotationMatrixInv * m_pos * -1;

	m_transMatrix = TransformationMatrix(m_lx.m_x, m_ly.m_x, m_lz.m_x, m_rot_pos.m_x,
										 m_lx.m_y, m_ly.m_y, m_lz.m_y, m_rot_pos.m_y,
										 m_lx.m_z, m_ly.m_z, m_lz.m_z, m_rot_pos.m_z,
										 0, 0, 0, 1);

	m_transMatrixInv = m_transMatrix.inverse();
}

PlaneNormal::PlaneNormal(const std::vector<IBKMK::Vector3D>& polygon) :
	m_valid(true),
	m_polygonPlane(Unknown),
	m_distance(0)
{
	IBK_ASSERT(polygon.size() > 2);

	m_pos = polygon[0];
	m_lx = polygon[1] - polygon[0];
	m_ly =  polygon.back() - polygon[0];
	m_lz = m_lx.crossProduct(m_ly);
	if (m_lz.magnitude() < 1e-9)
		m_valid = false;

	IBKMK::Vector3D n0 = m_lz * (1.0 / m_lz.magnitude());
	double d = m_pos.scalarProduct(n0);
	double fact = vectorDirection(n0);
	IBK_ASSERT(!IBK::near_zero(fact));

	m_distance = d * fact * -1.0;

	m_polygonPlane = SlopedPlane;

	int sameDirectionCount = 0;

	// now process all other points and generate their normal vectors as well
	for (unsigned int i=1; i<polygon.size(); ++i) {
		IBKMK::Vector3D vx = polygon[(i+1) % polygon.size()] - polygon[i];
		IBKMK::Vector3D vy = polygon[i-1] - polygon[i];
		IBKMK::Vector3D vn;
		vx.crossProduct(vy, vn);
		if (vn.magnitude() < 1e-9)
			return; // invalid vertex input
		vn.normalize();
		// adding reference normal to current vertexes normal and checking magnitude works
		if ((vn + m_lz).magnitude() > 1) // can be 0 or 2, so comparing against 1 is good even for rounding errors
			++sameDirectionCount;
		else
			--sameDirectionCount;
	}

	if (sameDirectionCount < 0) {
		// invert our normal vector
		m_lz *= -1;
	}

	m_ly = m_lz.crossProduct(m_lx);
	m_lx.normalize();
	m_ly.normalize();
	m_lz.normalize();

	m_rotationMatrix = RotationMatrix(m_lx.m_x, m_ly.m_x, m_lz.m_x,
									  m_lx.m_y, m_ly.m_y, m_lz.m_y,
									  m_lx.m_z, m_ly.m_z, m_lz.m_z);

	bool inverseValid = m_rotationMatrix.inverse(m_rotationMatrixInv);
	if(!inverseValid)
		m_valid = false;

	m_rot_pos = m_rotationMatrixInv * m_pos * -1;

	m_transMatrix = TransformationMatrix(m_lx.m_x, m_ly.m_x, m_lz.m_x, m_rot_pos.m_x,
										 m_lx.m_y, m_ly.m_y, m_lz.m_y, m_rot_pos.m_y,
										 m_lx.m_z, m_ly.m_z, m_lz.m_z, m_rot_pos.m_z,
										 0, 0, 0, 1);

	m_transMatrixInv = m_transMatrix.inverse();
}

IBKMK::Vector2D PlaneNormal::convert3DPoint(const IBKMK::Vector3D& point) const {

	if(m_polygonPlane != Unknown && m_polygonPlane != SlopedPlane) {
		IBKMK::Vector2D result;
		if(rotateVector(point, m_polygonPlane, result)) {
			IBKMK::Vector2D trans;
			rotateVector(m_pos, m_polygonPlane, trans);
			return result - trans;
		}
	}

	IBKMK::Vector3D tmp = m_rotationMatrixInv * point;
	tmp = tmp + m_rot_pos;

	return IBKMK::Vector2D(tmp.m_x, tmp.m_y);
}

IBKMK::Vector3D PlaneNormal::convert3DPointInv(const IBKMK::Vector2D& point) const {

	if(m_polygonPlane != Unknown && m_polygonPlane != SlopedPlane) {
		IBKMK::Vector2D trans;
		rotateVector(m_pos, m_polygonPlane, trans);
		IBKMK::Vector2D tmp = point + trans;
		IBKMK::Vector3D result;
		if(rotateVectorBack(tmp, m_polygonPlane, m_distance, result)) {
			return result;
		}
	}

	IBKMK::Vector3D res = IBKMK::Vector3D(point.m_x, point.m_y, 0) - m_rot_pos;
	res = m_rotationMatrix * res;

	return res;
}

PlaneNormal PlaneNormal::createXYPlane() {
	PlaneNormal res;
	res.m_pos = IBKMK::Vector3D(0,0,0);
	res.m_lx = IBKMK::Vector3D(1,0,0);
	res.m_ly = IBKMK::Vector3D(0,1,0);
	res.m_lz = IBKMK::Vector3D(0,0,1);
	return res;
}


void PlaneNormal::setPlaneProperties(const PlaneHesseNormal& plane) {
	double d = IBK::near_zero(plane.m_d) ? 1 : plane.m_d;
	double fact = vectorDirection(plane.m_n0);
	IBK_ASSERT(!IBK::near_zero(fact));

	m_distance = plane.m_d * fact * -1.0;
	IBKMK::Vector3D n = plane.m_n0 * d;
	m_lz = fact < 0 ? -1 * n : n;
	m_lz.normalize();

	if((IBK::near_equal(m_lz.m_x,1) || IBK::near_equal(m_lz.m_x,-1)) && IBK::near_zero(m_lz.m_y) && IBK::near_zero(m_lz.m_z))
		m_polygonPlane = YZPlane;
	else if(IBK::near_zero(m_lz.m_x) && (IBK::near_equal(m_lz.m_y,1) || IBK::near_equal(m_lz.m_y,-1)) && IBK::near_zero(m_lz.m_z))
		m_polygonPlane = XZPlane;
	else if(IBK::near_zero(m_lz.m_x) && IBK::near_zero(m_lz.m_y) && (IBK::near_equal(m_lz.m_z,1) || IBK::near_equal(m_lz.m_z,-1)))
		m_polygonPlane = XYPlane;
	else
		m_polygonPlane = SlopedPlane;
}

} // namespace IFCC
