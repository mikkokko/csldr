#include "pch.h"

void AnglesToMatrix(vec_t *angles, vec_t *forward, vec_t *side, vec_t *up)
{
	float yaw, pitch, roll;
	float sy, cy, sp, cp, sr, cr;

	yaw = RADIANS(angles[1]);
	pitch = RADIANS(angles[0]);
	roll = RADIANS(angles[2]);

	cy = cos(yaw);
	sy = sin(yaw);
	cp = cos(pitch);
	sp = sin(pitch);
	cr = cos(roll);
	sr = sin(roll);

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

/* mikkotodo are these from wikipedia */

void AnglesToQuat(vec_t *in, vec_t *out)
{
	float yaw, pitch, roll;
	float sy, cy, sp, cp, sr, cr;

	yaw = RADIANS(in[1]);
	pitch = RADIANS(in[0]);
	roll = RADIANS(in[2]);

	/* Abbreviations for the various angular functions */
	cy = cos(yaw * 0.5);
	sy = sin(yaw * 0.5);
	cp = cos(pitch * 0.5);
	sp = sin(pitch * 0.5);
	cr = cos(roll * 0.5);
	sr = sin(roll * 0.5);

	out[3] = cr * cp * cy + sr * sp * sy;
	out[0] = sr * cp * cy - cr * sp * sy;
	out[1] = cr * sp * cy + sr * cp * sy;
	out[2] = cr * cp * sy - sr * sp * cy;
}

/* mikkotodo cringe */
double CopySign(double x, double y)
{
    return y < 0 ? -fabs(x) : fabs(x);
}

void QuatToAngles(vec_t *in, vec_t *out)
{
	double sinr_cosp, cosr_cosp;
	double sinp;
	double siny_cosp, cosy_cosp;

	/* roll (x-axis rotation) */
	sinr_cosp = 2 * (in[3] * in[0] + in[1] * in[2]);
	cosr_cosp = 1 - 2 * (in[0] * in[0] + in[1] * in[1]);
	out[ROLL] = DEGREES(atan2(sinr_cosp, cosr_cosp));

	/* pitch (y-axis rotation) */
	sinp = 2 * (in[3] * in[1] - in[2] * in[0]);
	if (abs((int)sinp) >= 1) /* mikkotodo should this be absf? */
		out[PITCH] = DEGREES(CopySign(M_PI / 2, sinp)); /* use 90 degrees if out of range */
	else
		out[PITCH] = DEGREES(asin(sinp));

	/* yaw (z-axis rotation) */
	siny_cosp = 2 * (in[3] * in[2] + in[0] * in[1]);
	cosy_cosp = 1 - 2 * (in[1] * in[1] + in[2] * in[2]);
	out[YAW] = DEGREES(atan2(siny_cosp, cosy_cosp));
}

void QuatSlerp(vec_t *a, vec_t *b, float t, vec_t *out)
{
	float angle;
	float s1, s2;

	angle = acos(a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]);

	if (fabs(angle) >= 0.00001)
	{
		float sine;

		sine = sin(angle);
		s1 = sin((1 - t) * angle) / sine;
		s2 = sin(t * angle) / sine;
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
