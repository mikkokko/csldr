#include "pch.h"

extern cvar_t *fov_horplus;
extern cvar_t *fov_lerp;

float currentFov;

float fovDifference;

int destFov;
int initialFov;
float fovTime;
float fovLerp;

float HorPlusFov(float fov, float w, float h)
{
	return ATANF(TANF(fov / 360.0f * FL_PI) * (w / h * 0.75f)) / FL_PI * 360.0f;
}

void SetFov(float fov)
{
	SCREENINFO scr;

	fovDifference = fov / 90.0f;

	scr.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&scr);

	if (fov_horplus->value && ((scr.iHeight / scr.iWidth) != 0.75))
		/* user is not based and doesn't play in 4:3, do hor+ stuff */
		fov = HorPlusFov(fov, (float)scr.iWidth, (float)scr.iHeight);

	currentFov = fov;
}

void ForceDestFov(void)
{
	initialFov = destFov;
	SetFov((float)destFov);
}

void FovThink(void)
{
	float f, fov;

	if (destFov == initialFov)
		return;

	if (fovLerp == 0.0f)
	{
		ForceDestFov();
		return;
	}

	/* if some smart young man (me) disconnects and reconnects, don't break the fov */
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

	fov = Lerp((float)initialFov, (float)destFov, f);
	SetFov(fov);
}

void SetLerpFov(int fov, float lerp)
{
	if (destFov == fov)
		return;

	/* mikko: crappy clientside hack to fix those zoom in/out things before round
	start that look like they're straight from a frag movie, should remove */
	fovLerp = (fov == 0 || destFov == 0) ? 0 : lerp;

	initialFov = destFov;
	fovTime = clientTime;
	destFov = fov;
}

int (*Og_MsgFunc_SetFOV)(const char *pszName, int iSize, void *pbuf);

int Hk_MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	int fov = *(byte *)pbuf;

	SetLerpFov(fov, fov_lerp->value);
	return Og_MsgFunc_SetFOV(pszName, iSize, pbuf);
}
