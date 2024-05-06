#include <math.h>

#define F_PI (float)3.14159265358979323846

typedef float vec_t;
typedef float vec3_t[3];
typedef float mat3x4_t[3][4];

inline static int Rint(float x)
{
	return (int)((x < 0) ? (x - 0.5f) : (x + 0.5f));
}

inline static float Degrees(float rad)
{
	return rad * (180.0f / F_PI);
}

inline static float Radians(float deg)
{
	return deg * (F_PI / 180.0f);
}

inline static float DotProduct(const vec3_t a, const vec3_t b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline static float VectorLength(const vec3_t v)
{
	return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

inline static float VectorLength2D(const vec3_t v)
{
	return sqrtf(v[0] * v[0] + v[1] * v[1]);
}

inline static void VectorNormalize(vec3_t v)
{
	float len_sq = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (len_sq)
	{
		float s = 1 / sqrtf(len_sq);
		v[0] *= s;
		v[1] *= s;
		v[2] *= s;
	}
}

inline static void CrossProduct(const vec3_t a, const vec3_t b, vec3_t dst)
{
	dst[0] = a[1] * b[2] - a[2] * b[1];
	dst[1] = a[2] * b[0] - a[0] * b[2];
	dst[2] = a[0] * b[1] - a[1] * b[0];
}

inline static void VectorCopy(const vec3_t v, vec3_t dst)
{
	dst[0] = v[0];
	dst[1] = v[1];
	dst[2] = v[2];
}

inline static void VectorClear(vec3_t v)
{
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
}

inline static void VectorNegate(vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

inline static void VectorSubtract(const vec3_t a, const vec3_t b, vec3_t dst)
{
	dst[0] = a[0] - b[0];
	dst[1] = a[1] - b[1];
	dst[2] = a[2] - b[2];
}

inline static void VectorScale(const vec3_t a, float s, vec3_t dst)
{
	dst[0] = a[0] * s;
	dst[1] = a[1] * s;
	dst[2] = a[2] * s;
}


inline static void VectorMA(const vec3_t v, float s, vec3_t dst)
{
	dst[0] += v[0] * s;
	dst[1] += v[1] * s;
	dst[2] += v[2] * s;
}

inline static void VectorMA_2(const vec3_t v, float s, vec3_t dst)
{
	dst[0] += v[0] * s;
	dst[1] += v[1] * s;
	dst[2] -= v[2] * s;
}

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t side, vec3_t up);
void AngleLerp(const vec3_t a, const vec3_t b, float t, vec3_t dst);
