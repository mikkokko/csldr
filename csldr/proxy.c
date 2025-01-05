/* client.dll proxy */
#include "pch.h"

/* program version string, exported to make sure the compiler won't strip it */
#ifdef GIT_TAG
EXPORT const char *programVersion = "\ncsldr version:" GIT_TAG "\n";
#else
EXPORT const char *programVersion = "\ncsldr version:unknown\n";
#endif

static void *clientOrig;

/* bruh */
#define CLIENT_DLSYM_VALIDATE(variable, symbol) \
	*(void **)(&variable) = Plat_Dlsym(clientOrig, symbol); \
	if (variable == NULL) \
	{ \
		Plat_Error("Could not find " symbol "\n"); \
		return; \
	}

#define ORIG_SUFFIX "_orig"
#define ORIG_SUFFIX_LEN 5

void ProxyInit(void)
{
	char path[512];
	size_t length = Plat_CurrentModuleName(path, sizeof(path));
	if (!length)
	{
		Plat_Error("Could not get current module name\n");
		return;
	}

	/* need to be able to fit the suffix in there */
	if (length + ORIG_SUFFIX_LEN >= sizeof(path))
		Plat_Error("Game path is too long\n");

	char *end = path + length;
	char *ext = strrchr(path, '.'); /* no memrchr on msvc.. */
	if (!ext)
		ext = end; // no extension?

	size_t ext_length = end - ext;
	memmove(ext + ORIG_SUFFIX_LEN, ext, ext_length + 1); /* including the nul terminator */
	memcpy(ext, ORIG_SUFFIX, ORIG_SUFFIX_LEN);

	if (Secret_LoadClient(path))
		return;

	/* if this fails, it prints out why and terminates the process */
	clientOrig = Plat_CheckedDlopen(path);

	CLIENT_DLSYM_VALIDATE(pCreateInterface, "CreateInterface")

	/* don't use F because some clients won't call it
	CLIENT_DLSYM_VALIDATE(pF, "F") */

	CLIENT_DLSYM_VALIDATE(cl_funcs.pInitFunc, "Initialize")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudInitFunc, "HUD_Init")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudVidInitFunc, "HUD_VidInit")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudRedrawFunc, "HUD_Redraw")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudUpdateClientDataFunc, "HUD_UpdateClientData")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudResetFunc, "HUD_Reset")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientMove, "HUD_PlayerMove")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientMoveInit, "HUD_PlayerMoveInit")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientTextureType, "HUD_PlayerMoveTexture")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_ActivateMouse, "IN_ActivateMouse")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_DeactivateMouse, "IN_DeactivateMouse")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_MouseEvent, "IN_MouseEvent")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_ClearStates, "IN_ClearStates")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pIN_Accumulate, "IN_Accumulate")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCL_CreateMove, "CL_CreateMove")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCL_IsThirdPerson, "CL_IsThirdPerson")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCL_GetCameraOffsets, "CL_CameraOffset")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pFindKey, "KB_Find")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCamThink, "CAM_Think")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCalcRefdef, "V_CalcRefdef")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pAddEntity, "HUD_AddEntity")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pCreateEntities, "HUD_CreateEntities")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pDrawNormalTriangles, "HUD_DrawNormalTriangles")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pDrawTransparentTriangles, "HUD_DrawTransparentTriangles")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pStudioEvent, "HUD_StudioEvent")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pPostRunCmd, "HUD_PostRunCmd")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pShutdown, "HUD_Shutdown")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pTxferLocalOverrides, "HUD_TxferLocalOverrides")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pProcessPlayerState, "HUD_ProcessPlayerState")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pTxferPredictionData, "HUD_TxferPredictionData")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pReadDemoBuffer, "Demo_ReadBuffer")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pConnectionlessPacket, "HUD_ConnectionlessPacket")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pGetHullBounds, "HUD_GetHullBounds")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pHudFrame, "HUD_Frame")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pKeyEvent, "HUD_Key_Event")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pTempEntUpdate, "HUD_TempEntUpdate")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pGetUserEntity, "HUD_GetUserEntity")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pVoiceStatus, "HUD_VoiceStatus")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pDirectorMessage, "HUD_DirectorMessage")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pStudioInterface, "HUD_GetStudioModelInterface")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pChatInputPosition, "HUD_ChatInputPosition")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pGetPlayerTeam, "HUD_GetPlayerTeam")
	CLIENT_DLSYM_VALIDATE(cl_funcs.pClientFactory, "ClientFactory")
}

void ProxyQuit(void)
{
	Plat_Dlclose(clientOrig);
}
