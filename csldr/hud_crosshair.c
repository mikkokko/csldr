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

cvar_t *cl_crosshair_color;
cvar_t *cl_crosshair_translucent;

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

	/* string spoof */
	cl_crosshair_color = gEngfuncs.pfnGetCvarPointer("cl_crosshair_color");

	/* value spoof */
	cl_crosshair_translucent = gEngfuncs.pfnGetCvarPointer("cl_crosshair_translucent");

	can_xhair = (cl_crosshair_color && cl_crosshair_translucent);
}

int ScaleForRes(int value, int height)
{
	/* "default" resolution is 640x480 */
	return (int)((float)value * ((float)height / 480.0f));
}

void DrawQuad(int x0, int y0, int x1, int y1, float r, float g, float b)
{
	glDisable(GL_TEXTURE_2D);

	glColor3f(r, b, g);

	glBegin(GL_QUADS);
	glVertex2i(x0, y0);
	glVertex2i(x1, y0);
	glVertex2i(x1, y1);
	glVertex2i(x0, y1);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);
}

void DrawCrosshairSection(int x0, int y0, int x1, int y1)
{
	DrawQuad(x0, y0, x1, y1, xhair_color_r->value, xhair_color_b->value, xhair_color_g->value);
}

void DrawCrosshairPadding(int x0, int y0, int x1, int y1)
{
	int pad = (int)xhair_pad->value;

	DrawQuad(x0 - pad, y0 - pad, x1 + pad, y1 + pad, 0.0f, 0.0f, 0.0f);
}

void DrawCrosshair(void)
{
	SCREENINFO scr;
	int center_x, center_y;
	int gap, length, thickness;
	int y0, y1, x0, x1;
	wrect_t inner;
	wrect_t outer;

	/* dumb */
	if (!currentWeapon.m_iId || (currentWeapon.m_iId == WEAPON_SCOUT) ||
		(currentWeapon.m_iId == WEAPON_AWP) ||
		(currentWeapon.m_iId == WEAPON_G3SG1) ||
		(currentWeapon.m_iId == WEAPON_SG550))
		return;

	scr.iSize = sizeof(SCREENINFO);

	gEngfuncs.pfnGetScreenInfo(&scr);

	/* calculate coordinates */
	center_x = scr.iWidth / 2;
	center_y = scr.iHeight / 2;

	gap = ScaleForRes((int)xhair_gap->value, scr.iHeight);
	length = ScaleForRes((int)xhair_size->value, scr.iHeight);
	thickness = MAX(1, ScaleForRes((int)xhair_thick->value, scr.iHeight));

	inner.left = center_x - gap - thickness / 2;
	inner.right = inner.left + 2 * gap + thickness;
	inner.top = center_y - gap - thickness / 2;
	inner.bottom = inner.top + 2 * gap + thickness;

	outer.left = inner.left - length;
	outer.right = inner.right + length;
	outer.top = inner.top - length;
	outer.bottom = inner.bottom + length;

	y0 = center_y - thickness / 2;
	y1 = y0 + thickness;

	x0 = center_x - thickness / 2;
	x1 = x0 + thickness;

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

	if (xhair_dot->value)
		DrawCrosshairSection(x0, y0, x1, y1);

	if (!xhair_t->value)
		DrawCrosshairSection(x0, outer.top, x1, inner.top);

	DrawCrosshairSection(x0, inner.bottom, x1, outer.bottom);
	DrawCrosshairSection(outer.left, y0, inner.left, y1);
	DrawCrosshairSection(inner.right, y0, outer.right, y1);
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
	cl_crosshair_translucent->value = 1.0f;
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
