typedef struct tempent_s {
  int flags;
  float die;
  float frameMax;
  float x;
  float y;
  float z;
  float fadeSpeed;
  float bounceFactor;
  int hitSound;
  void (*hitcallback)(struct tempent_s*, pmtrace_t*);
  void (*callback)(struct tempent_s*, float, float);
  struct tempent_s* next;
  int priority;
  short clientIndex;
  Vector tentOffset;
  cl_entity_t entity;
} TEMPENTITY;

typedef struct
{
  particle_t* (*R_AllocParticle)(void (*)(particle_t*, float));
  void (*R_BlobExplosion)(float*);
  void (*R_Blood)(float*, float*, int, int);
  void (*R_BloodSprite)(float*, int, int, int, float);
  void (*R_BloodStream)(float*, float*, int, int);
  void (*R_BreakModel)(float*, float*, float*, float, float, int, int, char);
  void (*R_Bubbles)(float*, float*, float, int, int, float);
  void (*R_BubbleTrail)(float*, float*, float, int, int, float);
  void (*R_BulletImpactParticles)(float*);
  void (*R_EntityParticles)(cl_entity_t*);
  void (*R_Explosion)(float*, int, float, float, int);
  void (*R_FizzEffect)(cl_entity_t*, int, int);
  void (*R_FireField)(float*, int, int, int, int, float);
  void (*R_FlickerParticles)(float*);
  void (*R_FunnelSprite)(float*, int, int);
  void (*R_Implosion)(float*, float, int, float);
  void (*R_LargeFunnel)(float*, int);
  void (*R_LavaSplash)(float*);
  void (*R_MultiGunshot)(float*, float*, float*, int, int, int*);
  void (*R_MuzzleFlash)(float*, int);
  void (*R_ParticleBox)(float*, float*, unsigned char, unsigned char, unsigned char, float);
  void (*R_ParticleBurst)(float*, int, int, float);
  void (*R_ParticleExplosion)(float*);
  void (*R_ParticleExplosion2)(float*, int, int);
  void (*R_ParticleLine)(float*, float*, unsigned char, unsigned char, unsigned char, float);
  void (*R_PlayerSprites)(int, int, int, int);
  void (*R_Projectile)(float*, float*, int, int, int, void (*)(TEMPENTITY*, pmtrace_t*));
  void (*R_RicochetSound)(float*);
  void (*R_RicochetSprite)(float*, model_t*, float, float);
  void (*R_RocketFlare)(float*);
  void (*R_RocketTrail)(float*, float*, int);
  void (*R_RunParticleEffect)(float*, float*, int, int);
  void (*R_ShowLine)(float*, float*);
  void (*R_SparkEffect)(float*, int, int, int);
  void (*R_SparkShower)(float*);
  void (*R_SparkStreaks)(float*, int, int, int);
  void (*R_Spray)(float*, float*, int, int, int, int, int);
  void (*R_Sprite_Explode)(TEMPENTITY*, float, int);
  void (*R_Sprite_Smoke)(TEMPENTITY*, float);
  void (*R_Sprite_Spray)(float*, float*, int, int, int, int);
  void (*R_Sprite_Trail)(int, float*, float*, int, int, float, float, float, int, float);
  void (*R_Sprite_WallPuff)(TEMPENTITY*, float);
  void (*R_StreakSplash)(float*, float*, int, int, float, int, int);
  void (*R_TracerEffect)(float*, float*);
  void (*R_UserTracerParticle)(float*, float*, float, int, float, unsigned char, void (*)(particle_t*));
  particle_t* (*R_TracerParticles)(float*, float*, float);
  void (*R_TeleportSplash)(float*);
  void (*R_TempSphereModel)(float*, float, float, int, int);
  TEMPENTITY* (*R_TempModel)(float*, float*, float*, float, int, int);
  TEMPENTITY* (*R_DefaultSprite)(float*, int, float);
  TEMPENTITY* (*R_TempSprite)(float*, float*, float, int, int, int, float, float, int);
  int (*Draw_DecalIndex)(int);
  int (*Draw_DecalIndexFromName)(char*);
  void (*R_DecalShoot)(int, int, int, float*, int);
  void (*R_AttachTentToPlayer)(int, int, float, float);
  void (*R_KillAttachedTents)(int);
  BEAM* (*R_BeamCirclePoints)(int, float*, float*, int, float, float, float, float, float, int, float, float, float, float);
  BEAM* (*R_BeamEntPoint)(int, float*, int, float, float, float, float, float, int, float, float, float, float);
  BEAM* (*R_BeamEnts)(int, int, int, float, float, float, float, float, int, float, float, float, float);
  BEAM* (*R_BeamFollow)(int, int, float, float, float, float, float, float);
  void (*R_BeamKill)(int);
  BEAM* (*R_BeamLightning)(float*, float*, int, float, float, float, float, float);
  BEAM* (*R_BeamPoints)(float*, float*, int, float, float, float, float, float, int, float, float, float, float);
  BEAM* (*R_BeamRing)(int, int, int, float, float, float, float, float, int, float, float, float, float);
  dlight_t* (*CL_AllocDlight)(int);
  dlight_t* (*CL_AllocElight)(int);
  TEMPENTITY* (*CL_TempEntAlloc)(float*, model_t*);
  TEMPENTITY* (*CL_TempEntAllocNoModel)(float*);
  TEMPENTITY* (*CL_TempEntAllocHigh)(float*, model_t*);
  TEMPENTITY* (*CL_TentEntAllocCustom)(float*, model_t*, int, void (*)(TEMPENTITY*, float, float));
  void (*R_GetPackedColor)(short*, short);
  short (*R_LookupColor)(unsigned char, unsigned char, unsigned char);
  void (*R_DecalRemoveAll)(int);
  void (*R_FireCustomDecal)(int, int, int, float*, int, float);
} efx_api_t;
