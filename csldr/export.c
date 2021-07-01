/* all client.dll exported functions */
#include "pch.h"

EXPORT void CAM_Think(void)
{
	/* pass through */
	cl_funcs.pCamThink();
}

EXPORT void CL_CameraOffset(float *ofs)
{
	/* pass through */
	cl_funcs.pCL_GetCameraOffsets(ofs);
}

EXPORT void CL_CreateMove(float frametime, usercmd_t *cmd, int active)
{
	/* pass through */
	cl_funcs.pCL_CreateMove(frametime, cmd, active);
}

EXPORT int CL_IsThirdPerson(void)
{
	/* pass through */
	return cl_funcs.pCL_IsThirdPerson();
}

EXPORT void *ClientFactory(void)
{
	/* pass through */
	return cl_funcs.pClientFactory();
}

EXPORT void *CreateInterface(const char *pName, int *pReturnCode)
{
	/* pass through */
	return pCreateInterface(pName, pReturnCode);
}

EXPORT void Demo_ReadBuffer(int size, unsigned char *buffer)
{
	/* pass through */
	cl_funcs.pReadDemoBuffer(size, buffer);
}

EXPORT int HUD_AddEntity(int type, cl_entity_t *ent, const char *modelname)
{
	/* pass through */
	return cl_funcs.pAddEntity(type, ent, modelname);
}

EXPORT void HUD_ChatInputPosition(int *x, int *y)
{
	/* pass through */
	cl_funcs.pChatInputPosition(x, y);
}

EXPORT int HUD_ConnectionlessPacket(const netadr_t *net_from,
		const char *args,
		char *response_buffer,
		int *response_buffer_size)
{
	/* pass through */
	return cl_funcs.pConnectionlessPacket(net_from, args, response_buffer, response_buffer_size);
}

EXPORT void HUD_CreateEntities(void)
{
	/* pass through */
	cl_funcs.pCreateEntities();
}

EXPORT void HUD_DirectorMessage(int iSize, void *pbuf)
{
	/* pass through */
	cl_funcs.pDirectorMessage(iSize, pbuf);
}

EXPORT void HUD_DrawNormalTriangles(void)
{
	/* pass through */
	cl_funcs.pDrawNormalTriangles();
}

EXPORT void HUD_DrawTransparentTriangles(void)
{
	/* pass through */
	cl_funcs.pDrawTransparentTriangles();
}

EXPORT void HUD_Frame(double time)
{
	/* hooked */
	Hk_HudFrame(time);
}

EXPORT int HUD_GetHullBounds(int hullnumber, float *mins, float *maxs)
{
	/* pass through */
	return cl_funcs.pGetHullBounds(hullnumber, mins, maxs);
}

EXPORT int HUD_GetPlayerTeam(int iplayer)
{
	/* pass through */
	return cl_funcs.pGetPlayerTeam(iplayer);
}

EXPORT int HUD_GetStudioModelInterface(int version, r_studio_interface_t **ppinterface, engine_studio_api_t *pstudio)
{
	/* hooked */
	return Hk_GetStudioModelInterface(version, ppinterface, pstudio);
}

EXPORT cl_entity_t *HUD_GetUserEntity(int index)
{
	/* pass through */
	return cl_funcs.pGetUserEntity(index);
}

EXPORT void HUD_Init(void)
{
	/* hooked */
	Hk_HudInit();
}

EXPORT int HUD_Key_Event(int down, int keynum, const char *pszCurrentBinding)
{
	/* pass through */
	return cl_funcs.pKeyEvent(down, keynum, pszCurrentBinding);
}

EXPORT void HUD_PlayerMove(playermove_t *ppmove, int server)
{
	/* pass through */
	cl_funcs.pClientMove(ppmove, server);
}

EXPORT void HUD_PlayerMoveInit(playermove_t *ppmove)
{
	/* pass through */
	cl_funcs.pClientMoveInit(ppmove);
}

EXPORT char HUD_PlayerMoveTexture(char *name)
{
	/* pass through */
	return cl_funcs.pClientTextureType(name);
}

EXPORT void HUD_PostRunCmd(local_state_t *from,
		local_state_t *to,
		usercmd_t *cmd,
		int runfuncs,
		double time,
		unsigned int random_seed)
{
	/* hooked */
	Hk_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);
}

EXPORT void HUD_ProcessPlayerState(entity_state_t *dst, const entity_state_t *src)
{
	/* hooked */
	Hk_ProcessPlayerState(dst, src);
}

EXPORT int HUD_Redraw(float time, int intermission)
{
	/* hooked */
	return Hk_HudRedraw(time, intermission);
}

EXPORT void HUD_Reset(void)
{
	/* pass through */
	cl_funcs.pHudResetFunc();
}

EXPORT void HUD_Shutdown(void)
{
	/* hooked */
	Hk_HudShutdown();
}

EXPORT void HUD_StudioEvent(const mstudioevent_t *event, const cl_entity_t *entity)
{
	/* pass through */
	cl_funcs.pStudioEvent(event, entity);
}

EXPORT void HUD_TempEntUpdate(double frametime,
		double client_time,
		double cl_gravity,
		TEMPENTITY **ppTempEntFree,
		TEMPENTITY **ppTempEntActive,
		int (*Callback_AddVisibleEntity)(cl_entity_t *),
		void (*Callback_TempEntPlaySound)(TEMPENTITY *, float))
{
	/* pass through */
	cl_funcs.pTempEntUpdate(frametime,
			client_time,
			cl_gravity,
			ppTempEntFree,
			ppTempEntActive,
			Callback_AddVisibleEntity,
			Callback_TempEntPlaySound);
}

EXPORT void HUD_TxferLocalOverrides(entity_state_t *state, const clientdata_t *client)
{
	/* pass through */
	cl_funcs.pTxferLocalOverrides(state, client);
}

EXPORT void HUD_TxferPredictionData(entity_state_t *ps,
		const entity_state_t *pps,
		clientdata_t *pcd,
		const clientdata_t *ppcd,
		weapon_data_t *wd,
		const weapon_data_t *pwd)
{
	/* pass through */
	cl_funcs.pTxferPredictionData(ps, pps, pcd, ppcd, wd, pwd);
}

EXPORT int HUD_UpdateClientData(client_data_t *pcldata, float flTime)
{
	/* hooked */
	return Hk_UpdateClientData(pcldata, flTime);
}

EXPORT int HUD_VidInit(void)
{
	/* pass through */
	return cl_funcs.pHudVidInitFunc();
}

EXPORT void HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	/* pass through */
	cl_funcs.pVoiceStatus(entindex, bTalking);
}

EXPORT void IN_Accumulate(void)
{
	/* pass through */
	cl_funcs.pIN_Accumulate();
}

EXPORT void IN_ActivateMouse(void)
{
	/* pass through */
	cl_funcs.pIN_ActivateMouse();
}

EXPORT void IN_ClearStates(void)
{
	/* pass through */
	cl_funcs.pIN_ClearStates();
}

EXPORT void IN_DeactivateMouse(void)
{
	/* pass through */
	cl_funcs.pIN_DeactivateMouse();
}

EXPORT void IN_MouseEvent(int mstate)
{
	/* pass through */
	cl_funcs.pIN_MouseEvent(mstate);
}

EXPORT int Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion)
{
	/* hooked */
	return Hk_Initialize(pEnginefuncs, iVersion);
}

EXPORT kbutton_t *KB_Find(const char *name)
{
	/* pass through */
	return cl_funcs.pFindKey(name);
}

EXPORT void V_CalcRefdef(ref_params_t *pparams)
{
	/* hooked */
	Hk_CalcRefdef(pparams);
}
