#include "pch.h"

cl_enginefunc_t gEngfuncs;
cldll_func_t cl_funcs;

void *(*pCreateInterface)(const char *, int *);

bool isCzero;
bool isSoftware;

float clientTime;

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

	/* we have to know if software mode is enabled so we can disable all of the cool features */
#if defined(_WIN32)
	isSoftware = !GetModuleHandleA("hw.dll");
#else
	isSoftware = false;
#endif

	if (!isSoftware)
		GL_Load(); /* bad place for this */

	ViewInit();
	HudInit();
	InspectInit();
	ShellInit();
	CameraInit();

	/* is this czero */
	gamedir = gEngfuncs.pfnGetGameDirectory();

	if (!strcmp(gamedir, "czero"))
		isCzero = true;
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
