#include "pch.h"

cl_enginefunc_t gEngfuncs;
cldll_func_t cl_funcs;

void *(*pCreateInterface)(const char *, int *);

bool isCzero;

float clientTime;

cvar_t *cl_lw;

/*
-------------------------------------------------
 HookUserMsg hook
-------------------------------------------------
*/

int Hk_HookUserMsg(const char *szMsgName, pfnUserMsgHook pfn)
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

void Hk_HudFrame(double time)
{
	static bool sw_bitched;
	static bool lw_bitched;

	if (!isOpenGL && !sw_bitched)
	{
		gEngfuncs.Con_Printf("Not running in OpenGL mode. Some features of csldr will not work.\n");
		sw_bitched = true;
	}

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
