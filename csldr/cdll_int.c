#include "pch.h"

cl_enginefunc_t gEngfuncs;
cldll_func_t cl_funcs;

void *(*pCreateInterface)(const char *, int *);

bool isCzero;

float clientTime;

static cvar_t *cl_lw;

int screenWidth, screenHeight;

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

static int Hk_HookUserMsg(const char *szMsgName, pfnUserMsgHook pfn)
{
	pfnUserMsgHook hook = pfn;

	if (!strcmp(szMsgName, "SetFOV"))
	{
		Og_MsgFunc_SetFOV = pfn;
		hook = Hk_MsgFunc_SetFOV;
	}
	else if (!strcmp(szMsgName, "TeamInfo"))
	{
		Og_MsgFunc_TeamInfo = pfn;
		hook = Hk_MsgFunc_TeamInfo;
	}
	else if (!strcmp(szMsgName, "Brass"))
	{
		Og_MsgFunc_Brass = pfn;
		hook = Hk_MsgFunc_Brass;
	}
	else if (!strcmp(szMsgName, "CurWeapon"))
	{
		Og_MsgFunc_CurWeapon = pfn;
		hook = Hk_MsgFunc_CurWeapon;
	}
	else if (!strcmp(szMsgName, "HideWeapon"))
	{
		Og_MsgFunc_HideWeapon = pfn;
		hook = Hk_MsgFunc_HideWeapon;
	}

	return gEngfuncs.pfnHookUserMsg(szMsgName, hook);
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

	Og_TempModel = pEnginefuncs->pEfxAPI->R_TempModel;
	pEnginefuncs->pEfxAPI->R_TempModel = Hk_TempModel;

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
	
	Mem_Init();
	ViewInit();
	HudInit();
	InspectInit();
	ShellInit();
	CameraInit();
	GammaInit();

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
	Mem_Shutdown();
}

/*
-------------------------------------------------
 HUD Frame hook
-------------------------------------------------
*/

void Hk_HudFrame(double time)
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

#if defined(_WIN32)
	static bool warzone_bitched;

	if (isWarzone && !warzone_bitched)
	{
		warzone_bitched = true;
		const char *libName = (isWarzone == 2) ? "GTProtector.asi" : "GTLib.asi";
		gEngfuncs.Con_Printf("csldr found %s and removed it. The game might not work properly. Consider updating to the latest Steam version of the game.\n", libName);
	}
#endif

	UpdateScreenSize();

	GammaUpdate();

	// mikkotodo might be necessary again if we need extremely expensive tangent calc
	UpdateStudioCaches();

	clientTime = gEngfuncs.GetClientTime();
	cl_funcs.pHudFrame(time);
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
