typedef struct
{
  Vector normal;
  float dist;
} pmplane_t;

typedef struct
{
  qboolean allsolid;
  qboolean startsolid;
  qboolean inopen;
  qboolean inwater;
  float fraction;
  Vector endpos;
  pmplane_t plane;
  int ent;
  Vector deltavelocity;
  int hitgroup;
} pmtrace_t;
