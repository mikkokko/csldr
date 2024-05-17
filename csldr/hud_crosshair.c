#include "pch.h"

/* mikkotodo: other flags can hide the crosshair as well, however
the way those work is more complicated than just checking the flag
before drawing the crosshair */
#define HIDEHUD_CROSSHAIR (1 << 6)

static bool can_xhair;

static cvar_t *xhair_enable;

static cvar_t *xhair_gap;
static cvar_t *xhair_size;
static cvar_t *xhair_thick;
static cvar_t *xhair_pad;
static cvar_t *xhair_dot;
static cvar_t *xhair_t;
static cvar_t *xhair_dynamic_scale;
static cvar_t *xhair_gap_useweaponvalue;
static cvar_t *xhair_dynamic_move;

static cvar_t *xhair_color_r;
static cvar_t *xhair_color_g;
static cvar_t *xhair_color_b;
static cvar_t *xhair_alpha;

static cvar_t *cl_crosshair_color;
static cvar_t *cl_crosshair_translucent;
static cvar_t *hud_draw;

int currentWeaponId;
static int hideHudFlags;

// all of this is set by other code
int xhairPlayerFlags;
float xhairPlayerSpeed;
int xhairWeaponFlags;
int xhairShotsFired;

int (*Og_MsgFunc_CurWeapon)(const char *pszName, int iSize, void *pbuf);

int Hk_MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf)
{
	int state = ((byte *)pbuf)[0];
	int weaponId = ((char *)pbuf)[1];

	if (weaponId < 1)
		currentWeaponId = 0;
	else if (state)
		currentWeaponId = weaponId;

	return Og_MsgFunc_CurWeapon(pszName, iSize, pbuf);
}

int (*Og_MsgFunc_HideWeapon)(const char *pszName, int iSize, void *pbuf);

int Hk_MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf)
{
	hideHudFlags = ((byte *)pbuf)[0];
	return Og_MsgFunc_HideWeapon(pszName, iSize, pbuf);
}

void HudInit(void)
{
	CVAR_ARCHIVE_FAST(xhair_enable, 0);

	CVAR_ARCHIVE_FAST(xhair_gap, 0);
	CVAR_ARCHIVE_FAST(xhair_size, 4);
	CVAR_ARCHIVE_FAST(xhair_thick, 0);
	CVAR_ARCHIVE_FAST(xhair_pad, 0);
	CVAR_ARCHIVE_FAST(xhair_dot, 0);
	CVAR_ARCHIVE_FAST(xhair_t, 0);
	CVAR_ARCHIVE_FAST(xhair_dynamic_scale, 0);
	CVAR_ARCHIVE_FAST(xhair_gap_useweaponvalue, 0);
	CVAR_ARCHIVE_FAST(xhair_dynamic_move, 1);

	CVAR_ARCHIVE_FAST(xhair_color_r, 0);
	CVAR_ARCHIVE_FAST(xhair_color_g, 1);
	CVAR_ARCHIVE_FAST(xhair_color_b, 0);
	CVAR_ARCHIVE_FAST(xhair_alpha, 1);

	/* string spoof */
	cl_crosshair_color = gEngfuncs.pfnGetCvarPointer("cl_crosshair_color");

	/* value spoof */
	cl_crosshair_translucent = gEngfuncs.pfnGetCvarPointer("cl_crosshair_translucent");

	hud_draw = gEngfuncs.pfnGetCvarPointer("hud_draw");

	can_xhair = (cl_crosshair_color && cl_crosshair_translucent);
}

/* mikkotodo clean all of this up some day */

static int ScaleForRes(float value, int height)
{
	/* "default" resolution is 640x480 */
	return Rint(value * ((float)height / 480.0f));
}

/* d3d does not implement glVertex2fv!!! */
static void Vertex2fv(float *v)
{
	glVertex2f(v[0], v[1]);
}

static void DrawCrosshairSection(int _x0, int _y0, int _x1, int _y1)
{
	float x0 = (float)_x0;
	float y0 = (float)_y0;
	float x1 = (float)_x1;
	float y1 = (float)_y1;

	float top_left[2] = { x0, y0 };
	float top_right[2] = { x1, y0 };
	float bottom_right[2] = { x1, y1 };
	float bottom_left[2] = { x0, y1 };

	glColor4f(xhair_color_r->value,
		xhair_color_g->value,
		xhair_color_b->value,
		xhair_alpha->value);

	glBegin(GL_TRIANGLE_STRIP);
	Vertex2fv(top_left);
	Vertex2fv(bottom_left);
	Vertex2fv(top_right);
	Vertex2fv(bottom_right);
	glEnd();
}

static void DrawCrosshairPadding(int _pad, int _x0, int _y0, int _x1, int _y1)
{
	float pad = (float)_pad;
	float x0 = (float)_x0;
	float y0 = (float)_y0;
	float x1 = (float)_x1;
	float y1 = (float)_y1;

	float out_top_left[2] = { x0 - pad, y0 - pad };
	float out_top_right[2] = { x1 + pad, y0 - pad };
	float out_bottom_right[2] = { x1 + pad, y1 + pad };
	float out_bottom_left[2] = { x0 - pad, y1 + pad };
	float in_top_left[2] = { x0, y0 };
	float in_top_right[2] = { x1, y0 };
	float in_bottom_right[2] = { x1, y1 };
	float in_bottom_left[2] = { x0, y1 };

	glColor4f(0, 0, 0, xhair_alpha->value);

	glBegin(GL_TRIANGLE_STRIP);
	Vertex2fv(in_bottom_left);
	Vertex2fv(out_bottom_right);
	Vertex2fv(in_bottom_right);
	Vertex2fv(out_top_right);
	Vertex2fv(in_top_right);
	Vertex2fv(out_top_left);
	Vertex2fv(in_top_left);
	Vertex2fv(out_bottom_left);
	Vertex2fv(in_bottom_left);
	Vertex2fv(out_bottom_right);
	glEnd();
}

// for xhairPlayerFlags
#define FL_ONGROUND 0x200
#define FL_DUCKING 0x4000

enum
{
	ACCURACY_NONE = 0,
	ACCURACY_JUMP = (1 << 0),
	ACCURACY_RUN = (1 << 1),
	ACCURACY_DUCK = (1 << 2),
	ACCURACY_INACCURATE = (1 << 3),
	ACCURACY_VERY_INACCURATE = (1 << 4)
};

static int GetWeaponAccuracyFlags(int iWeaponID)
{
	switch (iWeaponID)
	{
	case WEAPON_P228:
	case WEAPON_FIVESEVEN:
	case WEAPON_DEAGLE:
		return (ACCURACY_DUCK | ACCURACY_RUN | ACCURACY_JUMP);

	case WEAPON_MAC10:
	case WEAPON_UMP45:
	case WEAPON_MP5N:
	case WEAPON_TMP:
		return ACCURACY_JUMP;

	case WEAPON_AUG:
	case WEAPON_GALIL:
	case WEAPON_M249:
	case WEAPON_SG552:
	case WEAPON_AK47:
	case WEAPON_P90:
		return (ACCURACY_RUN | ACCURACY_JUMP);

	case WEAPON_FAMAS:
		return (xhairWeaponFlags & 16) ? (ACCURACY_VERY_INACCURATE | ACCURACY_RUN | ACCURACY_JUMP) : (ACCURACY_RUN | ACCURACY_JUMP);

	case WEAPON_USP:
		return (xhairWeaponFlags & 1) ? (ACCURACY_INACCURATE | ACCURACY_DUCK | ACCURACY_RUN | ACCURACY_JUMP) : (ACCURACY_DUCK | ACCURACY_RUN | ACCURACY_JUMP);

	case WEAPON_GLOCK18:
		return (xhairWeaponFlags & 2) ? (ACCURACY_VERY_INACCURATE | ACCURACY_DUCK | ACCURACY_RUN | ACCURACY_JUMP) : (ACCURACY_DUCK | ACCURACY_RUN | ACCURACY_JUMP);

	case WEAPON_M4A1:
		return (xhairWeaponFlags & 4) ? (ACCURACY_INACCURATE | ACCURACY_RUN | ACCURACY_JUMP) : (ACCURACY_RUN | ACCURACY_JUMP);
	}

	return ACCURACY_NONE;
}

#define MAX_XHAIR_GAP 15

float GetCrosshairGap(void)
{
	static float xhairGap;
	static int lastShotsFired;
	static float xhairPrevTime;
	float minGap, deltaGap;

	switch (currentWeaponId)
	{
	case WEAPON_P228:
	case WEAPON_HEGRENADE:
	case WEAPON_SMOKEGRENADE:
	case WEAPON_FIVESEVEN:
	case WEAPON_USP:
	case WEAPON_GLOCK18:
	case WEAPON_AWP:
	case WEAPON_FLASHBANG:
	case WEAPON_DEAGLE:
		minGap = 8;
		deltaGap = 3;
		break;

	case WEAPON_SCOUT:
	case WEAPON_SG550:
	case WEAPON_SG552:
		minGap = 5;
		deltaGap = 3;
		break;

	case WEAPON_XM1014:
		minGap = 9;
		deltaGap = 4;
		break;

	case WEAPON_C4:
	case WEAPON_UMP45:
	case WEAPON_M249:
		minGap = 6;
		deltaGap = 3;
		break;

	case WEAPON_MAC10:
		minGap = 9;
		deltaGap = 3;
		break;

	case WEAPON_AUG:
		minGap = 3;
		deltaGap = 3;
		break;

	case WEAPON_MP5N:
		minGap = 6;
		deltaGap = 2;
		break;

	case WEAPON_M3:
		minGap = 8;
		deltaGap = 6;
		break;

	case WEAPON_TMP:
	case WEAPON_KNIFE:
	case WEAPON_P90:
		minGap = 7;
		deltaGap = 3;
		break;

	case WEAPON_G3SG1:
		minGap = 6;
		deltaGap = 4;
		break;

	case WEAPON_AK47:
		minGap = 4;
		deltaGap = 4;
		break;

	default:
		minGap = 4;
		deltaGap = 3;
		break;
	}

	if (!xhair_gap_useweaponvalue->value)
		minGap = 4;

	float baseMinGap = minGap;
	float absMinGap = baseMinGap * 0.5f;

	int flags = GetWeaponAccuracyFlags(currentWeaponId);
	if (xhair_dynamic_move->value && flags)
	{
		if (!(xhairPlayerFlags & FL_ONGROUND) && (flags & ACCURACY_JUMP))
		{
			minGap *= 2.0f;
		}
		else if ((xhairPlayerFlags & FL_DUCKING) && (flags & ACCURACY_DUCK))
		{
			minGap *= 0.5f;
		}
		else
		{
			float runLimit;

			switch (currentWeaponId)
			{
			case WEAPON_AUG:
			case WEAPON_GALIL:
			case WEAPON_FAMAS:
			case WEAPON_M249:
			case WEAPON_M4A1:
			case WEAPON_SG552:
			case WEAPON_AK47:
				runLimit = 140;
				break;

			case WEAPON_P90:
				runLimit = 170;
				break;

			default:
				runLimit = 0;
				break;
			}

			if (xhairPlayerSpeed > runLimit && (flags & ACCURACY_RUN))
				minGap *= 1.5f;
		}

		if (flags & ACCURACY_INACCURATE)
			minGap *= 1.4f;

		if (flags & ACCURACY_VERY_INACCURATE)
			minGap *= 1.4f;

		minGap = baseMinGap + (minGap - baseMinGap) * xhair_dynamic_scale->value;
		minGap = MAX(minGap, absMinGap);
	}

	if (xhairPrevTime > clientTime)
	{
		// client restart
		xhairPrevTime = clientTime;
	}
	
	float deltaTime = clientTime - xhairPrevTime;
	xhairPrevTime = clientTime;

	if (xhairShotsFired <= lastShotsFired)
	{
		// decay the crosshair as if we were always running at 100 fps
		xhairGap -= (100 * deltaTime) * (0.013f * xhairGap + 0.1f);
	}
	else
	{
		xhairGap += deltaGap * xhair_dynamic_scale->value;
		xhairGap = MIN(xhairGap, MAX_XHAIR_GAP);
	}

	if (xhairShotsFired > 600)
		xhairShotsFired = 1;

	lastShotsFired = xhairShotsFired;

	xhairGap = MAX(xhairGap, minGap);

	return xhairGap + xhair_gap->value;
}

static void DrawCrosshair(void)
{
	int center_x, center_y;
	int gap, length, thickness;
	int y0, y1, x0, x1;
	wrect_t inner;
	wrect_t outer;

	/* dumb */
	if (!currentWeaponId || (currentWeaponId == WEAPON_SCOUT) ||
		(currentWeaponId == WEAPON_AWP) ||
		(currentWeaponId == WEAPON_G3SG1) ||
		(currentWeaponId == WEAPON_SG550))
		return;

	/* calculate coordinates */
	center_x = screenWidth / 2;
	center_y = screenHeight / 2;

	gap = ScaleForRes(GetCrosshairGap(), screenHeight);
	length = ScaleForRes(xhair_size->value, screenHeight);
	thickness = ScaleForRes(xhair_thick->value, screenHeight);
	thickness = MAX(1, thickness);

	inner.left = (center_x - gap - thickness / 2);
	inner.right = (inner.left + 2 * gap + thickness);
	inner.top = (center_y - gap - thickness / 2);
	inner.bottom = (inner.top + 2 * gap + thickness);

	outer.left = (inner.left - length);
	outer.right = (inner.right + length);
	outer.top = (inner.top - length);
	outer.bottom = (inner.bottom + length);

	y0 = (center_y - thickness / 2);
	x0 = (center_x - thickness / 2);
	y1 = (y0 + thickness);
	x1 = (x0 + thickness);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (xhair_dot->value)
		DrawCrosshairSection(x0, y0, x1, y1);
	
	if (!xhair_t->value)
		DrawCrosshairSection(x0, outer.top, x1, inner.top);
	
	DrawCrosshairSection(x0, inner.bottom, x1, outer.bottom);
	DrawCrosshairSection(outer.left, y0, inner.left, y1);
	DrawCrosshairSection(inner.right, y0, outer.right, y1);

	/* draw padding if wanted */
	if (xhair_pad->value)
	{
		/* don't scale this */
		int pad = (int)xhair_pad->value;

		if (xhair_dot->value)
			DrawCrosshairPadding(pad, x0, y0, x1, y1);

		if (!xhair_t->value)
			DrawCrosshairPadding(pad, x0, outer.top, x1, inner.top);

		DrawCrosshairPadding(pad, x0, inner.bottom, x1, outer.bottom);
		DrawCrosshairPadding(pad, outer.left, y0, inner.left, y1);
		DrawCrosshairPadding(pad, inner.right, y0, outer.right, y1);
	}

	glColor4f(1, 1, 1, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}

int Hk_HudRedraw(float time, int intermission)
{
	char *color_str;
	float old_trans;
	char old_color[2];

	if (!canOpenGL
		|| !can_xhair
		|| !xhair_enable->value
		|| (hud_draw && !hud_draw->value)
		|| (hideHudFlags & HIDEHUD_CROSSHAIR))
	{
		return cl_funcs.pHudRedrawFunc(time, intermission);
	}

	/*  stupid hack, the memory is always writable though */
	color_str = (char *)cl_crosshair_color->string;

	/* back up the original values */
	old_trans = cl_crosshair_translucent->value;
	old_color[0] = color_str[0];
	old_color[1] = color_str[1];

	/* spoof the values so the crosshair will be invisible */
	cl_crosshair_translucent->value = 1;
	color_str[0] = '0'; /* the 0 as ascii character */
	color_str[1] = '\0'; /* the 0 as null terminator */

	cl_funcs.pHudRedrawFunc(time, intermission);

	/* restore the values */
	cl_crosshair_translucent->value = old_trans;
	color_str[0] = old_color[0];
	color_str[1] = old_color[1];

	/* draw our own crosshair */
	DrawCrosshair();

	return 1;
}

int Hk_HudVidInit(void)
{
	currentWeaponId = 0;
	hideHudFlags = 0;

	return cl_funcs.pHudVidInitFunc();
}
