#include "pch.h"

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t side, vec3_t up)
{
	float yaw, pitch, roll;
	float sy, cy, sp, cp, sr, cr;

	yaw = RADIANS(angles[1]);
	pitch = RADIANS(angles[0]);
	roll = RADIANS(angles[2]);

	cy = cosf(yaw);
	sy = sinf(yaw);
	cp = cosf(pitch);
	sp = sinf(pitch);
	cr = cosf(roll);
	sr = sinf(roll);

	if (forward)
	{
		forward[0] = cy * cp;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (side)
	{
		side[0] = cr * sy - cy * sp * sr;
		side[1] = -(sy * sp * sr + cy * cr);
		side[2] = -(cp * sr);
	}

	if (up)
	{
		up[0] = cy * cr * sp + sy * sr;
		up[1] = cr * sy * sp - cy * sr;
		up[2] = cp * cr;
	}
}

/* https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Source_code */
void AnglesToQuat(const vec3_t in, quat_t out)
{
	float yaw, pitch, roll;
	float sy, cy, sp, cp, sr, cr;

	yaw = RADIANS(in[1]);
	pitch = RADIANS(in[0]);
	roll = RADIANS(in[2]);

	cy = cosf(yaw * 0.5f);
	sy = sinf(yaw * 0.5f);
	cp = cosf(pitch * 0.5f);
	sp = sinf(pitch * 0.5f);
	cr = cosf(roll * 0.5f);
	sr = sinf(roll * 0.5f);

	out[3] = cr * cp * cy + sr * sp * sy;
	out[0] = sr * cp * cy - cr * sp * sy;
	out[1] = cr * sp * cy + sr * cp * sy;
	out[2] = cr * cp * sy - sr * sp * cy;
}

/* mikkotodo cringe */
static float CopySign(float x, float y)
{
	x = fabsf(x);
	return (y < 0) ? (-x) : (x);
}

/* https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Source_code_2 */
void QuatToAngles(const quat_t in, vec3_t out)
{
	float sinr_cosp, cosr_cosp;
	float sinp;
	float siny_cosp, cosy_cosp;

	sinr_cosp = 2 * (in[3] * in[0] + in[1] * in[2]);
	cosr_cosp = 1 - 2 * (in[0] * in[0] + in[1] * in[1]);
	out[ROLL] = DEGREES(atan2f(sinr_cosp, cosr_cosp));

	sinp = 2 * (in[3] * in[1] - in[2] * in[0]);
	if (fabsf(sinp) >= 1)
		out[PITCH] = DEGREES(CopySign(F_PI / 2, sinp));
	else
		out[PITCH] = DEGREES(asinf(sinp));

	siny_cosp = 2 * (in[3] * in[2] + in[0] * in[1]);
	cosy_cosp = 1 - 2 * (in[1] * in[1] + in[2] * in[2]);
	out[YAW] = DEGREES(atan2f(siny_cosp, cosy_cosp));
}

void QuatSlerp(const quat_t a, const quat_t b, float t, quat_t out)
{
	float angle;
	float s1, s2;

	angle = acosf(a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]);

	if (fabsf(angle) >= 0.00001f)
	{
		float sine;

		sine = sinf(angle);
		s1 = sinf((1 - t) * angle) / sine;
		s2 = sinf(t * angle) / sine;
	}
	else
	{
		s1 = t;
		s2 = 1.0f - s1;
	}

	out[0] = s1 * a[0] + s2 * b[0];
	out[1] = s1 * a[1] + s2 * b[1];
	out[2] = s1 * a[2] + s2 * b[2];
	out[3] = s1 * a[3] + s2 * b[3];
}

void SetIdentityMatrix(matrix3x4_t matrix)
{
	memset(matrix, 0, sizeof(*matrix));
	matrix[0][0] = 1;
	matrix[1][1] = 1;
	matrix[2][2] = 1;
}

void SetTranslationMatrix(const vec3_t translation, matrix3x4_t matrix)
{
	matrix[0][3] = translation[0];
	matrix[1][3] = translation[1];
	matrix[2][3] = translation[2];
}

void SetAngleMatrix(const vec3_t angles, matrix3x4_t matrix)
{
	float yaw, pitch, roll;
	float sr, sp, sy, cr, cp, cy;

	yaw = RADIANS(angles[1]);
	pitch = RADIANS(angles[0]);
	roll = RADIANS(angles[2]);

	sy = sinf(yaw);
	cy = cosf(yaw);
	sp = sinf(pitch);
	cp = cosf(pitch);
	sr = sinf(roll);
	cr = cosf(roll);

	float crcy = cr * cy;
	float crsy = cr * sy;
	float srcy = sr * cy;
	float srsy = sr * sy;

	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;

	matrix[0][1] = sp * srcy - crsy;
	matrix[1][1] = sp * srsy + crcy;
	matrix[2][1] = sr * cp;

	matrix[0][2] = (sp * crcy + srsy);
	matrix[1][2] = (sp * crsy - srcy);
	matrix[2][2] = cr * cp;

	matrix[0][3] = 0;
	matrix[1][3] = 0;
	matrix[2][3] = 0;
}

void GetMatrixTranslation(matrix3x4_t matrix, vec3_t translation)
{
	translation[0] = matrix[0][3];
	translation[1] = matrix[1][3];
	translation[2] = matrix[2][3];
}

void MatrixMultiply(matrix3x4_t in1, matrix3x4_t in2, matrix3x4_t out)
{
	if (in1 == out)
	{
		matrix3x4_t in1b;
		memcpy(in1b, in1, sizeof(matrix3x4_t));
		MatrixMultiply(in1b, in2, out);
		return;
	}

	if (in2 == out)
	{
		matrix3x4_t in2b;
		memcpy(in2b, in2, sizeof(matrix3x4_t));
		MatrixMultiply(in1, in2b, out);
		return;
	}

	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}
