#if defined(_WIN32)
#define _WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <GL/gl.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(a, b, c) (((a) > (c)) ? (c) : (((a) < (b)) ? (b) : (a)))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* bruh */
#define CVAR_ARHCIVE_FAST(name, value) \
	name = gEngfuncs.pfnRegisterVariable(#name, #value, FCVAR_ARCHIVE)

#include "../sdk/mathlib.h"
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
#include "../sdk/parsemsg.h"
#include "../sdk/cs_weapon.h"

#include "camera.h"
#include "cl_dll.h"
#include "fov.h"
#include "inspect.h"
#include "passthrough.h"
#include "platform.h"
#include "platform.h"
#include "player_info.h"
#include "shell.h"
#include "studiorenderer.h"
#include "view.h"
#include "weapon_info.h"
#include "secret_dll.h"

/* ui stuff */
#include "hud_crosshair.h"
