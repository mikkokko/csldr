#include <math.h>

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

#define F_PI (float)M_PI

#define PITCH 0
#define YAW 1
#define ROLL 2

typedef float vec3_t[3];

inline static float Degrees(float rad) { return (rad * (float)(180.0 / M_PI)); }
inline static float Radians(float deg) { return (deg * (float)(M_PI / 180.0)); }

inline static float VectorLength2D(const vec3_t v) { return sqrtf(v[0] * v[0] + v[1] * v[1]); }

inline static void VectorCopy(vec3_t dst, const vec3_t src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

inline static void VectorClear(vec3_t dst)
{
	dst[0] = 0;
	dst[1] = 0;
	dst[2] = 0;
}

inline static void VectorNegate(vec3_t dst)
{
	dst[0] = -dst[0];
	dst[1] = -dst[1];
	dst[2] = -dst[2];
}

inline static void VectorSubtract(const vec3_t a, const vec3_t b, vec3_t dst)
{
	dst[0] = a[0] - b[0];
	dst[1] = a[1] - b[1];
	dst[2] = a[2] - b[2];
}

inline static void VectorMA(vec3_t dst, vec3_t v, float s)
{
	dst[0] += v[0] * s;
	dst[1] += v[1] * s;
	dst[2] += v[2] * s;
} 

inline static void VectorMA_2(vec3_t dst, vec3_t v, float s)
{
	dst[0] += v[0] * s;
	dst[1] += v[1] * s;
	dst[2] -= v[2] * s;
}

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t side, vec3_t up);
void AngleLerp(const vec3_t a, const vec3_t b, float t, vec3_t out);
