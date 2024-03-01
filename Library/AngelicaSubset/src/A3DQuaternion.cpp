#include "A3DQuaternion.h"
#include "AAssist.h"
#include "A3DTypes.h"
#include "A3DVector.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////
//
// QuadToMatrix()
//
//	construct a rotation matrix with a quaternion
///////////////////////////////////////////////////////////////////////////////////////
void QuadToMatrix(const A3DQUATERNION& q, A3DMATRIX4& mat)
{
	// calculate coefficients
	float x2 = q.x + q.x;
	float y2 = q.y + q.y;
	float z2 = q.z + q.z;
	float xx = q.x * x2;
	float xy = q.x * y2;
	float xz = q.x * z2;
	float yy = q.y * y2;
	float yz = q.y * z2;
	float zz = q.z * z2;
	float wx = q.w * x2;
	float wy = q.w * y2;
	float wz = q.w * z2;

	mat._11 = 1.0f - (yy + zz);
	mat._21 = xy - wz;
	mat._31 = xz + wy;
	mat._41 = 0.0f;

	mat._12 = xy + wz;
	mat._22 = 1.0f - (xx + zz);
	mat._32 = yz - wx;
	mat._42 = 0.0f;

	mat._13 = xz - wy;
	mat._23 = yz + wx;
	mat._33 = 1.0f - (xx + yy);
	mat._43 = 0.0f;

	mat._14 = 0.0f;
	mat._24 = 0.0f;
	mat._34 = 0.0f;
	mat._44 = 1.0f;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// MatrixToQuad()
//
//	construct a quaternion with a rotating matrix. The matrix is not required to be a 
// pure rotation matrix, it can contains translation and scale factor, but the output
// quaternion will not be unit quaternion if it contains scale factor.
//
///////////////////////////////////////////////////////////////////////////////////////
void MatrixToQuad(const A3DMATRIX4& mat, A3DQUATERNION& q)
{
	float tr, s;

	// calculating the trace of the matrix, it is equal to 4(1 - x*x - y*y - z*z)=4w*w if it is a unit quaternion
	tr = mat._11 + mat._22 + mat._33 + 1.0f;
	
	// check the diagonal
	if (tr > 0.36f) // we can calculate out w directly
	{
		s = std::sqrtf(tr); // s is 2w
		q.w = s * 0.5f;
		s = 0.5f / s;	// now s is 1/4w
		q.x = (mat._23 - mat._32) * s;
		q.y = (mat._31 - mat._13) * s;
		q.z = (mat._12 - mat._21) * s;
	}
	else
	{
		// we have to calculate x, y or z first
		if (mat._11 >= mat._22 && mat._11 >= mat._33)
		{
			s = std::sqrtf(1.0f + mat._11 - mat._22 - mat._33); // s is 2x
			q.x = s * 0.5f;
			s = 0.5f / s;
			q.y = (mat._12 + mat._21) * s;
			q.z = (mat._13 + mat._31) * s;
			q.w = (mat._23 - mat._32) * s;
		}
		else if (mat._22 >= mat._11 && mat._22 >= mat._33)
		{
			s = std::sqrtf(1.0f + mat._22 - mat._11 - mat._33); // s is 2y
			q.y = s * 0.5f;
			s = 0.5f / s;
			q.x = (mat._12 + mat._21) * s;
			q.z = (mat._23 + mat._32) * s;
			q.w = (mat._31 - mat._13) * s;
		}
		else // mat._33 is maximum
		{
			s = std::sqrtf(1.0f + mat._33 - mat._11 - mat._22); // s is 2z
			q.z = s * 0.5f;
			s = 0.5f / s;
			q.x = (mat._13 + mat._31) * s;
			q.y = (mat._23 + mat._32) * s;
			q.w = (mat._12 - mat._21) * s;
		}
	}
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void QuadToEulerAngle(const A3DQUATERNION& q, FLOAT& vXAngle, FLOAT& vYAngle, FLOAT& vZAngle)
{
	// Roll (x-axis rotation)
	double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	vXAngle = static_cast<FLOAT>(atan2(sinr_cosp, cosr_cosp));

	// Pitch (y-axis rotation)
	double sinp = 2 * (q.w * q.y - q.z * q.x);
	if (std::fabs(sinp) >= 1)
		vYAngle = static_cast<FLOAT>(std::copysign(M_PI / 2, sinp)); // Use 90 degrees if out of range
	else
		vYAngle = static_cast<FLOAT>(asin(sinp));

	// Yaw (z-axis rotation)
	double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	vZAngle = static_cast<FLOAT>(atan2(siny_cosp, cosy_cosp));
}

void EulerAngleToQuad(FLOAT vXAngle, FLOAT vYAngle, FLOAT vZAngle, A3DQUATERNION& q)
{
	// We can convert each euler angle to a quaternion, than we multiply them together.
	double radXAngle = static_cast<double>(vXAngle) * 0.5;
	double radYAngle = static_cast<double>(vYAngle) * 0.5;
	double radZAngle = static_cast<double>(vZAngle) * 0.5;

	// calculate trig identities
	double cr = cos(radXAngle);
	double cp = cos(radYAngle);
	double cy = cos(radZAngle);

	double sr = sin(radXAngle);
	double sp = sin(radYAngle);
	double sy = sin(radZAngle);

	double cpcy = cp * cy;
	double spsy = sp * sy;
	double cpsy = cp * sy;
	double spcy = sp * cy;

	// Update quaternion components with the calculated values
	q.w = static_cast<FLOAT>(cr * cpcy + sr * spsy);
	q.x = static_cast<FLOAT>(sr * cpcy - cr * spsy);
	q.y = static_cast<FLOAT>(cr * spcy + sr * cpsy);
	q.z = static_cast<FLOAT>(cr * cpsy - sr * spcy);
}

void QuadToAxisAngle(const A3DQUATERNION& q, A3DVECTOR3& vecAxis, FLOAT& vAngle)
{
	float cosine, sine;

	cosine = q.w;
	vAngle = (std::acosf(cosine)) * 2.0f;
	sine = std::sqrtf(1.0f - cosine * cosine);

	if (sine < 0.0005f)
		sine = 1.0f;

	vecAxis.x = q.x / sine;
	vecAxis.y = q.y / sine;
	vecAxis.z = q.z / sine;
}

void AxisAngleToQuad(const A3DVECTOR3& vecAxis, FLOAT vAngle, A3DQUATERNION& q)
{
	// q = (cos(theta/2), x*sin(theta/2), y*sin(theta/2), z*sin(theta/2));
	float sine, cosine;

	vAngle *= 0.5f;
	sine = std::sinf(vAngle);
	cosine = std::cosf(vAngle);

	q.w = cosine;
	q.x = vecAxis.x * sine;
	q.y = vecAxis.y * sine;
	q.z = vecAxis.z * sine;
}

A3DQUATERNION SLERPQuad(const A3DQUATERNION& q1, const A3DQUATERNION& q2, float fraction)
{
	float sign;
	float f1, f2;
	float cosine = A3DQUATERNION::DotProduct(q1, q2);
	if (cosine < 0.0f)
	{
		cosine = -cosine;
		sign = -1.0f;
	}
	else
		sign = 1.0f;

	if (cosine > 1.0f - SLERP_EPSILON)
	{
		// the from and to value are very close, so use LERP will be ok
		f1 = 1.0f - fraction;
		f2 = fraction * sign;
	}
	else
	{
		float theta = std::acosf(cosine);
		float sine = std::sinf(theta);

		f1 = std::sinf((1.0f - fraction) * theta) / sine;
		f2 = (std::sinf(fraction * theta) / sine) * sign;
	}

	return q1 * f1 + q2 * f2;
}

float QuatDiffAngle(const A3DQUATERNION& q1, const A3DQUATERNION& q2)
{
	// we assume q can rotate q1 to q2, so we get
	// q1 * q = q2
	// q =  q1.Inverse() * q2
	// q.w = q1.w * q2.w - (-q1.x) * q2.x - (-q1.y) * q2.y - (-q1.z) * q2.z

	double fW = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;

	if (fW > 1.0 || fW < -1.0)
		return 0.0;

	double fDiffAngle = std::acos(fW) * 2.0;
	if (fDiffAngle > M_PI)
		return float(360 - fDiffAngle * 180 / M_PI);
	else
		return float(fDiffAngle * 180 / M_PI);
}
