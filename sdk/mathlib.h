#include <math.h>

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

#define FL_PI 3.14159265358979323846f

#if defined(_MSC_VER)
/* urgh */
#define atanf(x) (float)atan((double)x)
#define cosf(x) (float)cos((double)x)
#define fabsf(x) (float)fabs((double)x)
#define sinf(x) (float)sin((double)x)
#define sqrtf(x) (float)sqrt((double)x)
#define tanf(x) (float)tan((double)x)
#endif

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
