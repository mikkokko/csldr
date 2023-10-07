#include "pch.h"

// used to scale the viewmodel fov
float fovScale = 1;

static float currentFov = 90;
static float destFov = 90;
static float initialFov = 90;
static float fovTime;
static float fovLerp;

float GetCurrentFov(void)
{
	float w, h;

	w = (float)screenWidth;
	h = (float)screenHeight;

	if (fov_horplus->value && ((w / h) != 0.75f))
		return Degrees(atanf(tanf(Radians(currentFov) / 2) * (w / h * 0.75f))) * 2;
	else
		return currentFov;
}

static void SetFov(float fov)
{
	fovScale = fov / 90;
	currentFov = fov;
}

static void ForceDestFov(void)
{
	initialFov = destFov;
	SetFov(destFov);
}

static float FovInterp(float a, float b, float f)
{
	f -= 1;
	return (b - a) * sqrtf(1.0f - f * f) + a;
}

void FovThink(void)
{
	float f, fov;

	if ((int)destFov == (int)initialFov)
		return;

	if (!fovLerp)
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

	if (f >= 1)
	{
		ForceDestFov();
		return;
	}

	fov = FovInterp(initialFov, destFov, f);
	SetFov(fov);
}

static void SetLerpFov(float fov, float lerp)
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

	if (fov < 1 || fov > 180)
		fov = 90;

	SetLerpFov(fov, fov_lerp->value);
	return Og_MsgFunc_SetFOV(pszName, iSize, pbuf);
}
