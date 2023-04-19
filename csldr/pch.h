// avoid including windows.h by any means necessary
#ifdef _WIN32
#define WINAPI __stdcall
#define APIENTRY WINAPI
#endif
#include <glad/glad.h>


// mikkotodo fix once glad is compat
typedef unsigned int GLuint;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(a, b, c) (((a) > (c)) ? (c) : (((a) < (b)) ? (b) : (a)))

#define Q_OFFSETOF(s, m) ((size_t) &(((s *)0)->m))
#define Q_ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define STATIC_ASSERT(expr, msg) typedef int static_assert_##msg[(expr) ? (1) : (-1)];

/* mikkotodo move */
#if !defined(__cplusplus)
typedef enum {false, true} bool;
#endif
typedef int qboolean;

typedef unsigned char byte;

typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;

typedef struct
{
	byte r;
	byte g;
	byte b;
} color24;

typedef struct
{
	unsigned int r;
	unsigned int g;
	unsigned int b;
	unsigned int a;
} colorVec;

/* for sdk types that i cba to implement */
#define UNIMPLEMENTED_TYPE(name) typedef void name

/* mfw */
#define CVAR_ARCHIVE_FAST_STR(name, str) \
	name = gEngfuncs.pfnRegisterVariable(#name, str, FCVAR_ARCHIVE)

/* bruh */
#define CVAR_ARCHIVE_FAST(name, value) \
	CVAR_ARCHIVE_FAST_STR(name, #value)

#include "maths.h"
#include "msg.h"

#include "../sdk/weaponinfo.h"
#include "../sdk/entity_state.h"
#include "../sdk/com_model.h"
#include "../sdk/cl_entity.h"
#include "../sdk/cvar.h"
#include "../sdk/pm_movevars.h"
#include "../sdk/ref_params.h"
#include "../sdk/r_studioint.h"
#include "../sdk/r_efx.h"
#include "../sdk/event_api.h"
#include "../sdk/cdll_int.h"
#include "../sdk/studio.h"
#include "../sdk/cs_weapon.h"

#include "camera.h"
#include "cl_dll.h"
#include "fov.h"
#include "inspect.h"
#include "proxy.h"
#include "platform.h"
#include "player_info.h"
#include "shell.h"
#include "shader.h"
#include "studio_cache.h"
#include "studio_hook.h"
#include "studio_render.h"
#include "studiorenderer.h"
#include "view.h"
#include "weapon_info.h"
#include "secret_dll.h"
#include "memory.h"

/* ui stuff */
#include "hud_crosshair.h"
