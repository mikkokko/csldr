enum
{
  kRenderNormal = 0,
  kRenderTransColor = 1,
  kRenderTransTexture = 2,
  kRenderGlow = 3,
  kRenderTransAlpha = 4,
  kRenderTransAdd = 5
};

enum
{
  kRenderFxNone = 0,
  kRenderFxPulseSlow = 1,
  kRenderFxPulseFast = 2,
  kRenderFxPulseSlowWide = 3,
  kRenderFxPulseFastWide = 4,
  kRenderFxFadeSlow = 5,
  kRenderFxFadeFast = 6,
  kRenderFxSolidSlow = 7,
  kRenderFxSolidFast = 8,
  kRenderFxStrobeSlow = 9,
  kRenderFxStrobeFast = 10,
  kRenderFxStrobeFaster = 11,
  kRenderFxFlickerSlow = 12,
  kRenderFxFlickerFast = 13,
  kRenderFxNoDissipation = 14,
  kRenderFxDistort = 15,
  kRenderFxHologram = 16,
  kRenderFxDeadPlayer = 17,
  kRenderFxExplode = 18,
  kRenderFxGlowShell = 19,
  kRenderFxClampMinScale = 20,
  kRenderFxLightMultiplier = 21
};

typedef unsigned int string_t;

typedef unsigned char byte;

typedef int qboolean;

typedef struct
{
  byte r;
  byte g;
  byte b;
} color24;

typedef struct
{
  unsigned int r;
  unsigned int g;
  unsigned int b;
  unsigned int a;
} colorVec;

typedef struct link_s {
  struct link_s* prev;
  struct link_s* next;
} link_t;

typedef struct
{
  vec3_t normal;
  float dist;
} plane_t;

typedef struct
{
  qboolean allsolid;
  qboolean startsolid;
  qboolean inopen;
  qboolean inwater;
  float fraction;
  vec3_t endpos;
  plane_t plane;
  struct edict_s* ent;
  int hitgroup;
} trace_t;
