#include <math.h>

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

#define F_PI (float)M_PI

#define PITCH 0
#define YAW 1
#define ROLL 2

#define DEGREES(rad) (rad * (float)(180.0 / M_PI))
#define RADIANS(deg) (deg * (float)(M_PI / 180.0))

typedef float vec3_t[3];
typedef float quat_t[4];
typedef float matrix3x4_t[3][4];

#define VecLength2d(v) sqrtf(v[0] * v[0] + v[1] * v[1])

#define VecMultiplyAdd(dst, b, t) do { \
	dst[0] += t * b[0];\
	dst[1] += t * b[1];\
	dst[2] += t * b[2];\
} while (0)

/* bullshit */
#define VecMultiplyAdd2(dst, b, t) do { \
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

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t side, vec3_t up);
void AnglesToQuat(const vec3_t in, quat_t out);
void QuatToAngles(const quat_t in, vec3_t out);
void QuatSlerp(const quat_t a, const quat_t b, float t, quat_t out);

void SetIdentityMatrix(matrix3x4_t matrix);
void SetTranslationMatrix(const vec3_t translation, matrix3x4_t matrix);
void SetAngleMatrix(const vec3_t angles, matrix3x4_t matrix);
void GetMatrixTranslation(matrix3x4_t matrix, vec3_t translation);
void MatrixMultiply(matrix3x4_t in1, matrix3x4_t in2, matrix3x4_t out);
