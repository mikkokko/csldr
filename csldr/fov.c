#include "pch.h"

extern cvar_t *fov_horplus;
extern cvar_t *fov_lerp;

float fovDifference = 1.0f;

float currentFov = 90.0f;
float destFov = 90.0f;
float initialFov = 90.0f;
float fovTime;
float fovLerp;

float GetCurrentFov(void)
{
	float w, h;
	SCREENINFO scr;

	scr.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&scr);

	w = (float)scr.iWidth;
	h = (float)scr.iHeight;

	if (fov_horplus->value && ((w / h) != 0.75))
	{
		return atanf(tanf(currentFov / 360.0f * FL_PI) *
			(w / h * 0.75f)) / FL_PI * 360.0f;
	}
	else
		return currentFov;
}

void SetFov(float fov)
{
	fovDifference = fov / 90.0f;
	currentFov = fov;
}

void ForceDestFov(void)
{
	initialFov = destFov;
	SetFov(destFov);
}

static float FovInterp(float a, float b, float f)
{
	f -= 1.0f;
	return (b - a) * sqrtf(1.0f - f * f) + a;
}

void FovThink(void)
{
	float f, fov;

	if ((int)destFov == (int)initialFov)
		return;

	if (fovLerp == 0.0f)
	{
		ForceDestFov();
		return;
	}

	/* if player disconnects and reconnects, don't break the fov */
	if (clientTime < fovTime)
	{
		ForceDestFov();
		return;
	}

	f = (clientTime - fovTime) / fovLerp;

	if (f >= 1.0f)
	{
		ForceDestFov();
		return;
	}

	fov = FovInterp(initialFov, destFov, f);
	SetFov(fov);
}

void SetLerpFov(float fov, float lerp)
{
	if ((int)destFov == (int)fov)
		return;

	fovLerp = lerp;
	initialFov = destFov;
	fovTime = clientTime;
	destFov = fov;
}

int (*Og_MsgFunc_SetFOV)(const char *pszName, int iSize, void *pbuf);

int Hk_MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	float fov = (float)(*(byte *)pbuf);

	if (fov < 1.0f || fov > 180.0f)
		fov = 90.0f;

	SetLerpFov(fov, fov_lerp->value);
	return Og_MsgFunc_SetFOV(pszName, iSize, pbuf);
}
