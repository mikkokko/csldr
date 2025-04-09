#ifndef CSLDR_PCH_H
#define CSLDR_PCH_H

// avoid including windows.h by any means necessary
#ifdef _WIN32
#define WINAPI __stdcall
#define APIENTRY WINAPI
#endif
#include <glad/glad.h>

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
#include <setjmp.h>
#include <stddef.h>

/* mikkotodo move */
#if !defined(__cplusplus)
typedef enum {false, true} bool;
#endif

/* mfw */
#define CVAR_ARCHIVE_FAST_STR(name, str) \
	name = gEngfuncs.pfnRegisterVariable(#name, str, FCVAR_ARCHIVE)

/* bruh */
#define CVAR_ARCHIVE_FAST(name, value) \
	CVAR_ARCHIVE_FAST_STR(name, #value)

// mikkotodo fix
#define Vector vec3_t

// stupid hack to fix compiler warnings
#define sdk_string_const const

#include "maths.h"
#include "msg.h"

#include "sdk_include.h"

#include "camera.h"
#include "cl_dll.h"
#include "fov.h"
#include "inspect.h"
#include "keyvalue.h"
#include "proxy.h"
#include "platform.h"
#include "player_info.h"
#include "shell.h"
#include "studio_cache.h"
#include "studiorenderer.h"
#include "view.h"
#include "weapon_info.h"
#include "secret_dll.h"
#include "d3d.h"
#include "hash.h"

/* ui stuff */
#include "hud_crosshair.h"

/* the bloated renderer is in its own module, and is completely optional */
#include "render.h"

#endif // CSLDR_PCH_H
