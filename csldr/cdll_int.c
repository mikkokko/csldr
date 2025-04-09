#include "pch.h"

cl_enginefunc_t gEngfuncs;
cldll_func_t cl_funcs;

void *(*pCreateInterface)(const char *, int *);

bool isCzero;

float clientTime;

static cvar_t *cl_lw;

int screenWidth, screenHeight;

bool canOpenGL;

bool renderActive;

static void UpdateScreenSize(void)
{
	SCREENINFO scr;

	scr.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&scr);

	screenWidth = scr.iWidth;
	screenHeight = scr.iHeight;
}

/*
-------------------------------------------------
 HookUserMsg hook
-------------------------------------------------
*/

static pfnUserMsgHook HookUserMsg(const char *szMsgName, pfnUserMsgHook pfn)
{
#define CHECK_HOOK(name) \
	if (!Og_MsgFunc_##name && !strcmp(szMsgName, #name)) \
	{ \
		Og_MsgFunc_##name = pfn; \
		return Hk_MsgFunc_##name; \
	}

	CHECK_HOOK(SetFOV)
	CHECK_HOOK(TeamInfo)
	CHECK_HOOK(Brass)
	CHECK_HOOK(CurWeapon)
	CHECK_HOOK(HideWeapon)
	CHECK_HOOK(NVGToggle)

#undef CHECK_HOOK

	return pfn;
}

static int Hk_HookUserMsg(const char *szMsgName, pfnUserMsgHook pfn)
{
	return gEngfuncs.pfnHookUserMsg(szMsgName, HookUserMsg(szMsgName, pfn));
}

/*
-------------------------------------------------
 Client Initialize hook
-------------------------------------------------
*/

int Hk_Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion)
{
	/* back up original functions */
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t));

	/* install custom ones */
	pEnginefuncs->pfnHookUserMsg = Hk_HookUserMsg;
	pEnginefuncs->pfnHookEvent = Hk_HookEvent;
	pEnginefuncs->pfnFillRGBABlend = Hk_FillRGBABlend;

	Og_TempModel = pEnginefuncs->pEfxAPI->R_TempModel;
	pEnginefuncs->pEfxAPI->R_TempModel = Hk_TempModel;

	/* let the renderer tap in */
	Render_ModifyEngfuncs(pEnginefuncs);

	/* give the custom ones to client */
	return cl_funcs.pInitFunc(pEnginefuncs, iVersion);
}

/*
-------------------------------------------------
 HUD Initialize hook
-------------------------------------------------
*/

void Hk_HudInit(void)
{
	const char *gamedir;

	cl_funcs.pHudInitFunc();

	ViewInit();
	HudInit();
	InspectInit();
	ShellInit();
	CameraInit();

	/* is this czero */
	gamedir = gEngfuncs.pfnGetGameDirectory();

	if (!strcmp(gamedir, "czero"))
		isCzero = true;

	cl_lw = gEngfuncs.pfnGetCvarPointer("cl_lw");
}

/*
-------------------------------------------------
 HUD Shutdown hook
-------------------------------------------------
*/

void Hk_HudShutdown(void)
{
	cl_funcs.pShutdown();
}

/*
-------------------------------------------------
 HUD Frame hook
-------------------------------------------------
*/

static void BitchMessagesThink(void)
{
	static bool lw_bitched;

	if (cl_lw)
	{
		if (!cl_lw->value)
		{
			if (!lw_bitched)
			{
				gEngfuncs.Con_Printf("cl_lw is set to 0. Some features of csldr will not work.\n");
				lw_bitched = true;
			}
		}
		else if (lw_bitched)
			lw_bitched = false;
	}

	static bool opengl_bitched;

	if (!canOpenGL && !opengl_bitched)
	{
		opengl_bitched = true;
		gEngfuncs.Con_Printf("Failed to load OpenGL functions. Some features of csldr will not work.\n");
	}

#if defined(_WIN32)
	static bool warzone_bitched;

	if (isWarzone && !warzone_bitched)
	{
		warzone_bitched = true;
		const char *libName = (isWarzone == 2) ? "GTProtector.asi" : "GTLib.asi";
		gEngfuncs.Con_Printf("csldr found %s and removed it. The game might not work properly. Consider updating to the latest Steam version of the game.\n", libName);
	}
#endif
}

void Hk_HudFrame(double time)
{
	BitchMessagesThink();

	UpdateScreenSize();

	UpdateStudioCaches();

	clientTime = gEngfuncs.GetClientTime();
	cl_funcs.pHudFrame(time);

	renderActive = Render_BeginFrame();
}

/*
-------------------------------------------------
 UpdateClientData hook
-------------------------------------------------
*/

int Hk_UpdateClientData(client_data_t *pcldata, float flTime)
{
	cl_funcs.pHudUpdateClientDataFunc(pcldata, flTime);
	pcldata->fov = GetCurrentFov();
	return 1;
}
