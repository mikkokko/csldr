#include "pch.h"

static cvar_t *v_brightness;
static cvar_t *v_gamma;
static cvar_t *v_lightgamma;
static cvar_t *v_texgamma;

gammavars_t gammavars;

static void UpdateGammaVars(void);

void GammaInit(void)
{
	v_brightness = gEngfuncs.pfnGetCvarPointer("brightness");
	v_gamma = gEngfuncs.pfnGetCvarPointer("gamma");
	v_lightgamma = gEngfuncs.pfnGetCvarPointer("lightgamma");
	v_texgamma = gEngfuncs.pfnGetCvarPointer("texgamma");

	UpdateGammaVars();
}

void GammaUpdate(void)
{
	// check for change
	if (gammavars.brightness == v_brightness->value
		&& gammavars.gamma == v_gamma->value
		&& gammavars.lightgamma == v_lightgamma->value
		&& gammavars.texgamma == v_texgamma->value)
	{
		return;
	}

	UpdateGammaVars();
}

static void UpdateGammaVars(void)
{
	float brightness = v_brightness->value;
	float gamma = v_gamma->value;
	float lightgamma = v_lightgamma->value;
	float texgamma = v_texgamma->value;
	float g = gamma ? (1.0f / gamma) : 0.4f;
	float g1 = g * texgamma;

	gammavars.brightness = brightness;
	gammavars.gamma = gamma;
	gammavars.lightgamma = lightgamma;
	gammavars.texgamma = texgamma;
	gammavars.g = g;

	if (brightness <= 0)
		gammavars.g3 = 0.125f;
	else if (brightness > 1)
		gammavars.g3 = 0.05f;
	else
		gammavars.g3 = 0.125f - brightness * brightness * 0.075f;

	for (int i = 0; i < 256; i++)
	{
		float f = (float)i / 255.0f;

		int color = (int)(255.0f * powf(f, g1));
		gammavars.textable[i] = (byte)CLAMP(color, 0, 255);

		color = (int)(255.0f * powf(f, gamma));
		gammavars.lineartable[i] = (byte)CLAMP(color, 0, 255);
	}
}
