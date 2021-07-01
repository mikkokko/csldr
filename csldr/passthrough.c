/* client.dll passthrough */
#include "pch.h"

void *clientOrig;

/* bruh */
#define CLIENT_DLSYM_VALIDATE(variable, symbol) \
	*(void **)(&variable) = Plat_Dlsym(clientOrig, symbol); \
	if (variable == NULL) \
	{ \
		Plat_Error("Could not find " symbol "\n"); \
		return; \
	}

void PassInit(void)
{
	char name[256];
	char *dot;

	Plat_CurrentModuleName(name, sizeof(name));

	dot = strrchr(name, '.');

	if (dot == NULL)
	{
		Plat_Error("Could not get current module name\n");
		return;
	}

	/* mikkotodo unsafe */
	strcpy(dot, "_orig" LIB_EXT);

	clientOrig = Plat_Dlopen(name);

	if (clientOrig == NULL)
	{
		Plat_Error("Could not load client_orig" LIB_EXT "\n");
		return;
	}

	CLIENT_DLSYM_VALIDATE(pCreateInterface, "CreateInterface");

	/* don't use F because some clients won't call it
	CLIENT_DLSYM_VALIDATE(pF, "F"); */

	CLIENT_DLSYM_VALIDATE(cl_funcs.pInitFunc, "Initialize");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudInitFunc, "HUD_Init");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudVidInitFunc, "HUD_VidInit");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudRedrawFunc, "HUD_Redraw");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudUpdateClientDataFunc, "HUD_UpdateClientData");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudResetFunc, "HUD_Reset");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientMove, "HUD_PlayerMove");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientMoveInit, "HUD_PlayerMoveInit");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientTextureType, "HUD_PlayerMoveTexture");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_ActivateMouse, "IN_ActivateMouse");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_DeactivateMouse, "IN_DeactivateMouse");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_MouseEvent, "IN_MouseEvent");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_ClearStates, "IN_ClearStates");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_Accumulate, "IN_Accumulate");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCL_CreateMove, "CL_CreateMove");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCL_IsThirdPerson, "CL_IsThirdPerson");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCL_GetCameraOffsets, "CL_CameraOffset");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pFindKey, "KB_Find");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCamThink, "CAM_Think");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCalcRefdef, "V_CalcRefdef");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pAddEntity, "HUD_AddEntity");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCreateEntities, "HUD_CreateEntities");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pDrawNormalTriangles, "HUD_DrawNormalTriangles");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pDrawTransparentTriangles, "HUD_DrawTransparentTriangles");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pStudioEvent, "HUD_StudioEvent");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pPostRunCmd, "HUD_PostRunCmd");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pShutdown, "HUD_Shutdown");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pTxferLocalOverrides, "HUD_TxferLocalOverrides");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pProcessPlayerState, "HUD_ProcessPlayerState");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pTxferPredictionData, "HUD_TxferPredictionData");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pReadDemoBuffer, "Demo_ReadBuffer");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pConnectionlessPacket, "HUD_ConnectionlessPacket");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pGetHullBounds, "HUD_GetHullBounds");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudFrame, "HUD_Frame");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pKeyEvent, "HUD_Key_Event");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pTempEntUpdate, "HUD_TempEntUpdate");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pGetUserEntity, "HUD_GetUserEntity");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pVoiceStatus, "HUD_VoiceStatus");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pDirectorMessage, "HUD_DirectorMessage");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pStudioInterface, "HUD_GetStudioModelInterface");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pChatInputPosition, "HUD_ChatInputPosition");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pGetPlayerTeam, "HUD_GetPlayerTeam");
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientFactory, "ClientFactory");
}

void PassQuit(void)
{
	Plat_Dlclose(clientOrig);
}
