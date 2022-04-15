#include "mathlib.h"

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = RAD(angles[YAW]);
	sy = sin(angle);
	cy = cos(angle);
	angle = RAD(angles[PITCH]);
	sp = sin(angle);
	cp = cos(angle);
	angle = RAD(angles[ROLL]);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (right)
	{
		right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
		right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
		right[2] = -1 * sr * cp;
	}

	if (up)
	{
		up[0] = (cr * sp * cy + -sr * -sy);
		up[1] = (cr * sp * sy + -sr * cy);
		up[2] = cr * cp;
	}
}

void VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}

void InverseRollVectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] - scale * vecb[2]; /*  bruh */
}

float Lerp(float a, float b, float f)
{
	return a * (1.0f - f) + b * f;
}

void VectorLerp(vec3_t a, vec3_t b, float f, vec3_t out)
{
	out[0] = a[0] * (1.0f - f) + b[0] * f;
	out[1] = a[1] * (1.0f - f) + b[1] * f;
	out[2] = a[2] * (1.0f - f) + b[2] * f;
}
