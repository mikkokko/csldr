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
