#include <math.h>

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

#define PITCH 0
#define YAW 1
#define ROLL 2

#define DEGREES(rad) (float)(rad * (180.0 / M_PI))
#define RADIANS(deg) (float)(deg * (M_PI / 180.0))

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

#define Vec2_Length(v) sqrt(v[0] * v[0] + v[1] * v[1])

#define Vec3_MulAdd(dst, b, t) do { \
	dst[0] += t * b[0];\
	dst[1] += t * b[1];\
	dst[2] += t * b[2];\
} while (0)

/* bullshit */
#define Vec3_MulAddAlt(dst, b, t) do { \
	dst[0] += t * b[0];\
	dst[1] += t * b[1];\
	dst[2] -= t * b[2];\
} while (0)

#define QuatCopy(dst, src) do { \
	dst[0] = src[0]; \
	dst[1] = src[1]; \
	dst[2] = src[2]; \
	dst[3] = src[3]; \
} while(0)

#define QuatClear(dst) do { \
	dst[0] = 0; \
	dst[1] = 0; \
	dst[2] = 0; \
	dst[3] = 0; \
} while(0)


void AnglesToMatrix(vec_t * angles, vec_t * forward, vec_t * side, vec_t * up);
void AnglesToQuat(vec_t * in, vec_t * out);
void QuatToAngles(vec_t *in, vec_t *out);
void QuatSlerp(vec_t *a, vec_t *b, float t, vec_t * out);
