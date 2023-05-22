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

static cvar_t *xhair_color_r;
static cvar_t *xhair_color_g;
static cvar_t *xhair_color_b;
static cvar_t *xhair_alpha;

static cvar_t *cl_crosshair_color;
static cvar_t *cl_crosshair_translucent;
static cvar_t *hud_draw;

int currentWeaponId;
static int hideHudFlags;

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

	CVAR_ARCHIVE_FAST(xhair_gap, 4);
	CVAR_ARCHIVE_FAST(xhair_size, 4);
	CVAR_ARCHIVE_FAST(xhair_thick, 0);
	CVAR_ARCHIVE_FAST(xhair_pad, 0);
	CVAR_ARCHIVE_FAST(xhair_dot, 0);
	CVAR_ARCHIVE_FAST(xhair_t, 0);

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
	glVertex2fv(top_left);
	glVertex2fv(bottom_left);
	glVertex2fv(top_right);
	glVertex2fv(bottom_right);
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
	glVertex2fv(in_bottom_left);
	glVertex2fv(out_bottom_right);
	glVertex2fv(in_bottom_right);
	glVertex2fv(out_top_right);
	glVertex2fv(in_top_right);
	glVertex2fv(out_top_left);
	glVertex2fv(in_top_left);
	glVertex2fv(out_bottom_left);
	glVertex2fv(in_bottom_left);
	glVertex2fv(out_bottom_right);
	glEnd();
}

static void DrawCrosshair(void)
{
	SCREENINFO scr;
	int width, height;
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

	scr.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&scr);
	width = scr.iWidth;
	height = scr.iHeight;

	/* calculate coordinates */
	center_x = width / 2;
	center_y = height / 2;

	gap = ScaleForRes(xhair_gap->value, height);
	length = ScaleForRes(xhair_size->value, height);
	thickness = ScaleForRes(xhair_thick->value, height);
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

	if (!can_xhair
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
