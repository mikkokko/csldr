UNIMPLEMENTED_TYPE(player_info_t);
UNIMPLEMENTED_TYPE(alight_t);

typedef struct
{
	void *(*Mem_Calloc)(int, size_t);
	void *(*Cache_Check)(cache_user_t *);
	void (*LoadCacheFile)(const char *, cache_user_t *);
	model_t *(*Mod_ForName)(const char *, int);
	void *(*Mod_Extradata)(model_t *);
	model_t *(*GetModelByIndex)(int);
	cl_entity_t *(*GetCurrentEntity)();
	player_info_t *(*PlayerInfo)(int);
	entity_state_t *(*GetPlayerState)(int);
	cl_entity_t *(*GetViewEntity)();
	void (*GetTimes)(int *, double *, double *);
	cvar_t *(*GetCvar)(const char *);
	void (*GetViewInfo)(float *, float *, float *, float *);
	model_t *(*GetChromeSprite)();
	void (*GetModelCounters)(int **, int **);
	void (*GetAliasScale)(float *, float *);
	float ****(*StudioGetBoneTransform)();
	float ****(*StudioGetLightTransform)();
	float ***(*StudioGetAliasTransform)();
	float ***(*StudioGetRotationMatrix)();
	void (*StudioSetupModel)(int, void **, void **);
	int (*StudioCheckBBox)();
	void (*StudioDynamicLight)(cl_entity_t *, alight_t *);
	void (*StudioEntityLight)(alight_t *);
	void (*StudioSetupLighting)(alight_t *);
	void (*StudioDrawPoints)();
	void (*StudioDrawHulls)();
	void (*StudioDrawAbsBBox)();
	void (*StudioDrawBones)();
	void (*StudioSetupSkin)(void *, int);
	void (*StudioSetRemapColors)(int, int);
	model_t *(*SetupPlayerModel)(int);
	void (*StudioClientEvents)();
	int (*GetForceFaceFlags)();
	void (*SetForceFaceFlags)(int);
	void (*StudioSetHeader)(void *);
	void (*SetRenderModel)(model_t *);
	void (*SetupRenderer)(int);
	void (*RestoreRenderer)();
	void (*SetChromeOrigin)();
	int (*IsHardware)();
	void (*GL_StudioDrawShadow)();
	void (*GL_SetRenderMode)(int);
	void (*StudioSetRenderamt)(int);
	void (*StudioSetCullState)(int);
	void (*StudioRenderShadow)(int, float *, float *, float *, float *);
} engine_studio_api_t;

typedef struct
{
	int version;
	int (*StudioDrawModel)(int);
	int (*StudioDrawPlayer)(int, entity_state_t *);
} r_studio_interface_t;
