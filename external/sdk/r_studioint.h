typedef struct
{
  void* (*Mem_Calloc)(int, size_t);
  void* (*Cache_Check)(cache_user_t*);
  void (*LoadCacheFile)(char*, cache_user_t*);
  model_t* (*Mod_ForName)(const char*, int);
  void* (*Mod_Extradata)(model_t*);
  model_t* (*GetModelByIndex)(int);
  cl_entity_t* (*GetCurrentEntity)(void);
  player_info_t* (*PlayerInfo)(int);
  entity_state_t* (*GetPlayerState)(int);
  cl_entity_t* (*GetViewEntity)(void);
  void (*GetTimes)(int*, double*, double*);
  cvar_t* (*GetCvar)(const char*);
  void (*GetViewInfo)(float*, float*, float*, float*);
  model_t* (*GetChromeSprite)(void);
  void (*GetModelCounters)(int**, int**);
  void (*GetAliasScale)(float*, float*);
  float**** (*StudioGetBoneTransform)(void);
  float**** (*StudioGetLightTransform)(void);
  float*** (*StudioGetAliasTransform)(void);
  float*** (*StudioGetRotationMatrix)(void);
  void (*StudioSetupModel)(int, void**, void**);
  int (*StudioCheckBBox)(void);
  void (*StudioDynamicLight)(cl_entity_t*, alight_t*);
  void (*StudioEntityLight)(alight_t*);
  void (*StudioSetupLighting)(alight_t*);
  void (*StudioDrawPoints)(void);
  void (*StudioDrawHulls)(void);
  void (*StudioDrawAbsBBox)(void);
  void (*StudioDrawBones)(void);
  void (*StudioSetupSkin)(void*, int);
  void (*StudioSetRemapColors)(int, int);
  model_t* (*SetupPlayerModel)(int);
  void (*StudioClientEvents)(void);
  int (*GetForceFaceFlags)(void);
  void (*SetForceFaceFlags)(int);
  void (*StudioSetHeader)(void*);
  void (*SetRenderModel)(model_t*);
  void (*SetupRenderer)(int);
  void (*RestoreRenderer)(void);
  void (*SetChromeOrigin)(void);
  int (*IsHardware)(void);
  void (*GL_StudioDrawShadow)(void);
  void (*GL_SetRenderMode)(int);
  void (*StudioSetRenderamt)(int);
  void (*StudioSetCullState)(int);
  void (*StudioRenderShadow)(int, float*, float*, float*, float*);
} engine_studio_api_t;

typedef struct
{
  int version;
  int (*StudioDrawModel)(int);
  int (*StudioDrawPlayer)(int, entity_state_t*);
} r_studio_interface_t;
