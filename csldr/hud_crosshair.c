#include "pch.h"

bool can_xhair;

cvar_t *xhair_enable;

cvar_t *xhair_gap;
cvar_t *xhair_size;
cvar_t *xhair_thick;
cvar_t *xhair_pad;
cvar_t *xhair_dot;
cvar_t *xhair_t;

cvar_t *xhair_color_r;
cvar_t *xhair_color_g;
cvar_t *xhair_color_b;
cvar_t *xhair_alpha;

cvar_t *cl_crosshair_color;
cvar_t *cl_crosshair_translucent;

int currentWeaponId;

int (*Og_MsgFunc_CurWeapon)(const char *pszName, int iSize, void *pbuf);

int Hk_MsgFunc_CurWeapon(const char *name, int size, void *data)
{
	int state = ((byte *)data)[0];
	int weaponId = ((char *)data)[1];

	if (weaponId < 1)
		currentWeaponId = 0;
	else if (state)
		currentWeaponId = weaponId;

	return Og_MsgFunc_CurWeapon(name,size,data);
}

void HudInit(void)
{
	CVAR_ARHCIVE_FAST(xhair_enable, 0);

	CVAR_ARHCIVE_FAST(xhair_gap, 4);
	CVAR_ARHCIVE_FAST(xhair_size, 4);
	CVAR_ARHCIVE_FAST(xhair_thick, 0);
	CVAR_ARHCIVE_FAST(xhair_pad, 0);
	CVAR_ARHCIVE_FAST(xhair_dot, 0);
	CVAR_ARHCIVE_FAST(xhair_t, 0);

	CVAR_ARHCIVE_FAST(xhair_color_r, 0);
	CVAR_ARHCIVE_FAST(xhair_color_g, 1);
	CVAR_ARHCIVE_FAST(xhair_color_b, 0);
	CVAR_ARHCIVE_FAST(xhair_alpha, 1);

	/* string spoof */
	cl_crosshair_color = gEngfuncs.pfnGetCvarPointer("cl_crosshair_color");

	/* value spoof */
	cl_crosshair_translucent = gEngfuncs.pfnGetCvarPointer("cl_crosshair_translucent");

	can_xhair = (cl_crosshair_color && cl_crosshair_translucent);
}

/* mikkotodo clean all of this up some day */

static float ScaleForRes(float value, float height)
{
	/* "default" resolution is 640x480 */
	return value * (height / 480);
}

static void DrawCrosshairSection(float x0, float y0, float x1, float y1)
{
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

static void DrawCrosshairPadding(float x0, float y0, float x1, float y1)
{
	float pad = roundf(xhair_pad->value);

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

typedef struct
{
	float left;
	float right;
	float top;
	float bottom;
} frect_t;

static void DrawCrosshair(void)
{
	SCREENINFO scr;
	float width, height;
	float center_x, center_y;
	float gap, length, thickness;
	float y0, y1, x0, x1;
	frect_t inner;
	frect_t outer;

	/* dumb */
	if (!currentWeaponId || (currentWeaponId == WEAPON_SCOUT) ||
		(currentWeaponId == WEAPON_AWP) ||
		(currentWeaponId == WEAPON_G3SG1) ||
		(currentWeaponId == WEAPON_SG550))
		return;

	scr.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&scr);
	width = (float)scr.iWidth;
	height = (float)scr.iHeight;

	/* calculate coordinates */
	center_x = width / 2;
	center_y = height / 2;

	gap = ScaleForRes(xhair_gap->value, height);
	length = ScaleForRes(xhair_size->value, height);
	thickness = ScaleForRes(xhair_thick->value, height);
	thickness = MAX(1, thickness);

	inner.left = roundf(center_x - gap - thickness / 2);
	inner.right = roundf(inner.left + 2 * gap + thickness);
	inner.top = roundf(center_y - gap - thickness / 2);
	inner.bottom = roundf(inner.top + 2 * gap + thickness);

	outer.left = roundf(inner.left - length);
	outer.right = roundf(inner.right + length);
	outer.top = roundf(inner.top - length);
	outer.bottom = roundf(inner.bottom + length);

	y0 = roundf(center_y - thickness / 2);
	x0 = roundf(center_x - thickness / 2);
	y1 = roundf(y0 + thickness);
	x1 = roundf(x0 + thickness);

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
		if (xhair_dot->value)
			DrawCrosshairPadding(x0, y0, x1, y1);

		if (!xhair_t->value)
			DrawCrosshairPadding(x0, outer.top, x1, inner.top);

		DrawCrosshairPadding(x0, inner.bottom, x1, outer.bottom);
		DrawCrosshairPadding(outer.left, y0, inner.left, y1);
		DrawCrosshairPadding(inner.right, y0, outer.right, y1);
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

	if (!isOpenGL || !can_xhair || !xhair_enable->value)
		return cl_funcs.pHudRedrawFunc(time, intermission);

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
