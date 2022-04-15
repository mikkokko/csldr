#include <math.h>

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

#define DEG(rad) (float)(rad * (180.0 / M_PI))
#define RAD(deg) (float)(deg * (M_PI / 180.0))

#define PITCH 0
#define YAW 1
#define ROLL 2

typedef float vec_t;
typedef vec_t vec3_t[3];

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc);
void InverseRollVectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc);

float Lerp(float a, float b, float f);
void VectorLerp(vec3_t a, vec3_t b, float f, vec3_t out);
