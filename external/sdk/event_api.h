typedef struct
{
  int version;
  void (*EV_PlaySound)(int, float*, int, const char*, float, float, int, int);
  void (*EV_StopSound)(int, int, const char*);
  int (*EV_FindModelIndex)(const char*);
  int (*EV_IsLocal)(int);
  int (*EV_LocalPlayerDucking)(void);
  void (*EV_LocalPlayerViewheight)(float*);
  void (*EV_LocalPlayerBounds)(int, float*, float*);
  int (*EV_IndexFromTrace)(pmtrace_t*);
  physent_t* (*EV_GetPhysent)(int);
  void (*EV_SetUpPlayerPrediction)(int, int);
  void (*EV_PushPMStates)(void);
  void (*EV_PopPMStates)(void);
  void (*EV_SetSolidPlayers)(int);
  void (*EV_SetTraceHull)(int);
  void (*EV_PlayerTrace)(float*, float*, int, int, pmtrace_t*);
  void (*EV_WeaponAnimation)(int, int);
  unsigned short (*EV_PrecacheEvent)(int, const char*);
  void (*EV_PlaybackEvent)(int, const edict_t*, unsigned short, float, float*, float*, float, float, int, int, int, int);
  const char* (*EV_TraceTexture)(int, float*, float*);
  void (*EV_StopAllSounds)(int, int);
  void (*EV_KillEvents)(int, const char*);
} event_api_t;
