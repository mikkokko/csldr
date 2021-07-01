UNIMPLEMENTED_TYPE(client_sprite_t);
UNIMPLEMENTED_TYPE(hud_player_info_t);
UNIMPLEMENTED_TYPE(client_textmessage_t);
UNIMPLEMENTED_TYPE(event_args_t);
UNIMPLEMENTED_TYPE(screenfade_t);
UNIMPLEMENTED_TYPE(triangleapi_t);
UNIMPLEMENTED_TYPE(demo_api_t);
UNIMPLEMENTED_TYPE(net_api_t);
UNIMPLEMENTED_TYPE(IVoiceTweak_t);
UNIMPLEMENTED_TYPE(sequenceEntry_t);
UNIMPLEMENTED_TYPE(sentenceEntry_t);
UNIMPLEMENTED_TYPE(cmdalias_t);
UNIMPLEMENTED_TYPE(con_nprint_t);
#if !defined(_WIN32) || !defined(__cplusplus)
UNIMPLEMENTED_TYPE(tagPOINT);
#endif

UNIMPLEMENTED_TYPE(playermove_t);
UNIMPLEMENTED_TYPE(kbutton_t);
UNIMPLEMENTED_TYPE(mstudioevent_t);
UNIMPLEMENTED_TYPE(netadr_t);

/* ??? */
typedef struct
{
	int left;
	int right;
	int top;
	int bottom;
} wrect_t;

typedef int (*pfnUserMsgHook)(const char *, int iSize, void *);

/* was HSPRITE but that collided with windows sdk */
typedef int hsprite_t;

typedef struct
{
	int iSize;
	int iWidth;
	int iHeight;
	int iFlags;
	int iCharHeight;
	short charWidths[256];
} SCREENINFO;

typedef struct
{
	vec3_t origin;
	vec3_t viewangles;
	int iWeaponBits;
	float fov;
} client_data_t;

typedef struct
{
	hsprite_t (*pfnSPR_Load)(const char *);
	int (*pfnSPR_Frames)(hsprite_t);
	int (*pfnSPR_Height)(hsprite_t, int);
	int (*pfnSPR_Width)(hsprite_t, int);
	void (*pfnSPR_Set)(hsprite_t, int, int, int);
	void (*pfnSPR_Draw)(int, int, int, const wrect_t *);
	void (*pfnSPR_DrawHoles)(int, int, int, const wrect_t *);
	void (*pfnSPR_DrawAdditive)(int, int, int, const wrect_t *);
	void (*pfnSPR_EnableScissor)(int, int, int, int);
	void (*pfnSPR_DisableScissor)();
	client_sprite_t *(*pfnSPR_GetList)(const char *, int *);
	void (*pfnFillRGBA)(int, int, int, int, int, int, int, int);
	int (*pfnGetScreenInfo)(SCREENINFO *);
	void (*pfnSetCrosshair)(hsprite_t, wrect_t, int, int, int);
	cvar_t *(*pfnRegisterVariable)(const char *, const char *, int);
	float (*pfnGetCvarFloat)(const char *);
	const char *(*pfnGetCvarString)(const char *);
	int (*pfnAddCommand)(const char *, void (*)());
	int (*pfnHookUserMsg)(const char *, pfnUserMsgHook);
	int (*pfnServerCmd)(const char *);
	int (*pfnClientCmd)(const char *);
	void (*pfnGetPlayerInfo)(int, hud_player_info_t *);
	void (*pfnPlaySoundByName)(const char *, float);
	void (*pfnPlaySoundByIndex)(int, float);
	void (*pfnAngleVectors)(const float *, float *, float *, float *);
	client_textmessage_t *(*pfnTextMessageGet)(const char *);
	int (*pfnDrawCharacter)(int, int, int, int, int, int);
	int (*pfnDrawConsoleString)(int, int, const char *);
	void (*pfnDrawSetTextColor)(float, float, float);
	void (*pfnDrawConsoleStringLen)(const char *, int *, int *);
	void (*pfnConsolePrint)(const char *);
	void (*pfnCenterPrint)(const char *);
	int (*GetWindowCenterX)();
	int (*GetWindowCenterY)();
	void (*GetViewAngles)(float *);
	void (*SetViewAngles)(float *);
	int (*GetMaxClients)();
	void (*Cvar_SetValue)(const char *, float);
	int (*Cmd_Argc)();
	const char *(*Cmd_Argv)(int);
	void (*Con_Printf)(const char *, ...);
	void (*Con_DPrintf)(const char *, ...);
	void (*Con_NPrintf)(int, const char *, ...);
	void (*Con_NXPrintf)(con_nprint_t *, const char *, ...);
	const char *(*PhysInfo_ValueForKey)(const char *);
	const char *(*ServerInfo_ValueForKey)(const char *);
	float (*GetClientMaxspeed)();
	int (*CheckParm)(const char *, const char **);
	void (*Key_Event)(int, int);
	void (*GetMousePosition)(int *, int *);
	int (*IsNoClipping)();
	cl_entity_t *(*GetLocalPlayer)();
	cl_entity_t *(*GetViewModel)();
	cl_entity_t *(*GetEntityByIndex)(int);
	float (*GetClientTime)();
	void (*V_CalcShake)();
	void (*V_ApplyShake)(float *, float *, float);
	int (*PM_PointContents)(float *, int *);
	int (*PM_WaterEntity)(float *);
	pmtrace_t *(*PM_TraceLine)(float *, float *, int, int, int);
	model_t *(*CL_LoadModel)(const char *, int *);
	int (*CL_CreateVisibleEntity)(int, cl_entity_t *);
	const model_t *(*GetSpritePointer)(hsprite_t);
	void (*pfnPlaySoundByNameAtLocation)(const char *, float, float *);
	unsigned short (*pfnPrecacheEvent)(int, const char *);
	void (*pfnPlaybackEvent)(int, const edict_t *, unsigned short, float, float *, float *, float, float, int, int, int,
			int);
	void (*pfnWeaponAnim)(int, int);
	float (*pfnRandomFloat)(float, float);
	int32 (*pfnRandomLong)(int32, int32);
	void (*pfnHookEvent)(const char *, void (*)(event_args_t *));
	int (*Con_IsVisible)();
	const char *(*pfnGetGameDirectory)();
	cvar_t *(*pfnGetCvarPointer)(const char *);
	const char *(*Key_LookupBinding)(const char *);
	const char *(*pfnGetLevelName)();
	void (*pfnGetScreenFade)(screenfade_t *);
	void (*pfnSetScreenFade)(screenfade_t *);
	void *(*VGui_GetPanel)();
	void (*VGui_ViewportPaintBackground)(int *);
	byte *(*COM_LoadFile)(const char *, int, int *);
	const char *(*COM_ParseFile)(const char *, const char *);
	void (*COM_FreeFile)(void *);
	triangleapi_t *pTriAPI;
	efx_api_t *pEfxAPI;
	event_api_t *pEventAPI;
	demo_api_t *pDemoAPI;
	net_api_t *pNetAPI;
	IVoiceTweak_t *pVoiceTweak;
	int (*IsSpectateOnly)();
	model_t *(*LoadMapSprite)(const char *);
	void (*COM_AddAppDirectoryToSearchPath)(const char *, const char *);
	int (*COM_ExpandFilename)(const char *, const char *, int);
	const char *(*PlayerInfo_ValueForKey)(int, const char *);
	void (*PlayerInfo_SetValueForKey)(const char *, const char *);
	qboolean (*GetPlayerUniqueID)(int, const char *);
	int (*GetTrackerIDForPlayer)(int);
	int (*GetPlayerForTrackerID)(int);
	int (*pfnServerCmdUnreliable)(const char *);
	void (*GetMousePos)(tagPOINT *);
	void (*SetMousePos)(int, int);
	void (*SetMouseEnable)(qboolean);
	cvar_t *(*GetFirstCVarPtr)();
	unsigned int (*GetFirstCmdFunctionHandle)();
	unsigned int (*GetNextCmdFunctionHandle)(unsigned int);
	const char *(*GetCmdFunctionName)(unsigned int);
	float (*GetClientOldTime)();
	float (*GetServerGravityValue)();
	model_t *(*GetModelByIndex)(int);
	void (*pfnSetFilterMode)(int);
	void (*pfnSetFilterColor)(float, float, float);
	void (*pfnSetFilterBrightness)(float);
	sequenceEntry_t *(*pfnSequenceGet)(const char *, const char *);
	void (*pfnSPR_DrawGeneric)(int, int, int, const wrect_t *, int, int, int, int);
	sentenceEntry_t *(*pfnSequencePickSentence)(const char *, int, int *);
	int (*pfnDrawString)(int, int, const char *, int, int, int);
	int (*pfnDrawStringReverse)(int, int, const char *, int, int, int);
	const char *(*LocalPlayerInfo_ValueForKey)(const char *);
	int (*pfnVGUI2DrawCharacter)(int, int, int, unsigned int);
	int (*pfnVGUI2DrawCharacterAdd)(int, int, int, int, int, int, unsigned int);
	unsigned int (*COM_GetApproxWavePlayLength)(const char *);
	void *(*pfnGetCareerUI)();
	void (*Cvar_Set)(const char *, const char *);
	int (*pfnIsPlayingCareerMatch)();
	void (*pfnPlaySoundVoiceByName)(const char *, float, int);
	void (*pfnPrimeMusicStream)(const char *, int);
	double (*GetAbsoluteTime)();
	void (*pfnProcessTutorMessageDecayBuffer)(int *, int);
	void (*pfnConstructTutorMessageDecayBuffer)(int *, int);
	void (*pfnResetTutorMessageDecayData)();
	void (*pfnPlaySoundByNameAtPitch)(const char *, float, int);
	void (*pfnFillRGBABlend)(int, int, int, int, int, int, int, int);
	int (*pfnGetAppID)();
	cmdalias_t *(*pfnGetAliases)();
	void (*pfnVguiWrap2_GetMouseDelta)(int *, int *);
	int (*pfnFilteredClientCmd)(const char *);
} cl_enginefunc_t;

typedef struct
{
	int (*pInitFunc)(cl_enginefunc_t *, int);
	void (*pHudInitFunc)();
	int (*pHudVidInitFunc)();
	int (*pHudRedrawFunc)(float, int);
	int (*pHudUpdateClientDataFunc)(client_data_t *, float);
	void (*pHudResetFunc)();
	void (*pClientMove)(playermove_t *, qboolean);
	void (*pClientMoveInit)(playermove_t *);
	char (*pClientTextureType)(const char *);
	void (*pIN_ActivateMouse)();
	void (*pIN_DeactivateMouse)();
	void (*pIN_MouseEvent)(int);
	void (*pIN_ClearStates)();
	void (*pIN_Accumulate)();
	void (*pCL_CreateMove)(float, usercmd_t *, int);
	int (*pCL_IsThirdPerson)();
	void (*pCL_GetCameraOffsets)(float *);
	kbutton_t *(*pFindKey)(const char *);
	void (*pCamThink)();
	void (*pCalcRefdef)(ref_params_t *);
	int (*pAddEntity)(int, cl_entity_t *, const char *);
	void (*pCreateEntities)();
	void (*pDrawNormalTriangles)();
	void (*pDrawTransparentTriangles)();
	void (*pStudioEvent)(const mstudioevent_t *, const cl_entity_t *);
	void (*pPostRunCmd)(local_state_t *, local_state_t *, usercmd_t *, int, double, unsigned int);
	void (*pShutdown)();
	void (*pTxferLocalOverrides)(entity_state_t *, const clientdata_t *);
	void (*pProcessPlayerState)(entity_state_t *, const entity_state_t *);
	void (*pTxferPredictionData)(entity_state_t *, const entity_state_t *, clientdata_t *, const clientdata_t *,
			weapon_data_t *, const weapon_data_t *);
	void (*pReadDemoBuffer)(int, unsigned const char *);
	int (*pConnectionlessPacket)(const netadr_t *, const char *, const char *, int *);
	int (*pGetHullBounds)(int, float *, float *);
	void (*pHudFrame)(double);
	int (*pKeyEvent)(int, int, const char *);
	void (*pTempEntUpdate)(double, double, double, tempent_t **, tempent_t **, int (*)(cl_entity_t *),
			void (*)(tempent_t *, float));
	cl_entity_t *(*pGetUserEntity)(int);
	void (*pVoiceStatus)(int, qboolean);
	void (*pDirectorMessage)(int, void *);
	int (*pStudioInterface)(int, r_studio_interface_t **, engine_studio_api_t *);
	void (*pChatInputPosition)(int *, int *);
	int (*pGetPlayerTeam)(int);
	void *(*pClientFactory)();
} cldll_func_t;
