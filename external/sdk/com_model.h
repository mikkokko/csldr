typedef enum {
  mod_brush = 0,
  mod_sprite = 1,
  mod_alias = 2,
  mod_studio = 3
} modtype_t;

typedef enum {
  ST_SYNC = 0,
  ST_RAND = 1
} synctype_t;

typedef struct
{
  float mins[3];
  float maxs[3];
  float origin[3];
  int headnode[4];
  int visleafs;
  int firstface;
  int numfaces;
} dmodel_t;

typedef struct
{
  Vector normal;
  float dist;
  byte type;
  byte signbits;
  unsigned char pad[2];
} mplane_t;

typedef struct
{
  Vector position;
} mvertex_t;

typedef struct
{
  unsigned short v[2];
  unsigned int cachededgeoffset;
} medge_t;

typedef struct texture_s {
  char name[16];
  unsigned int width;
  unsigned int height;
  int anim_total;
  int anim_min;
  int anim_max;
  struct texture_s* anim_next;
  struct texture_s* alternate_anims;
  unsigned int offsets[4];
  unsigned int paloffset;
} texture_t;

typedef struct
{
  float vecs[2][4];
  float mipadjust;
  texture_t* texture;
  int flags;
} mtexinfo_t;

typedef struct mnode_s {
  int contents;
  int visframe;
  short minmaxs[6];
  struct mnode_s* parent;
  mplane_t* plane;
  struct mnode_s* children[2];
  unsigned short firstsurface;
  unsigned short numsurfaces;
} mnode_t;

typedef struct decal_s {
  struct decal_sdecal_s* pnext;
  struct msurface_s* psurface;
  short dx;
  short dy;
  short texture;
  byte scale;
  byte flags;
  short entityIndex;
} decal_t;

typedef struct
{
  int contents;
  int visframe;
  short minmaxs[6];
  mnode_t* parent;
  byte* compressed_vis;
  struct efrag_s* efrags;
  struct msurface_s** firstmarksurface;
  int nummarksurfaces;
  int key;
  unsigned char ambient_sound_level[4];
} mleaf_t;

typedef struct msurface_s {
  int visframe;
  int dlightframe;
  int dlightbits;
  mplane_t* plane;
  int flags;
  int firstedge;
  int numedges;
  struct surfcache_s* cachespots[4];
  short texturemins[2];
  short extents[2];
  mtexinfo_t* texinfo;
  unsigned char styles[4];
  color24* samples;
  decal_t* pdecals;
} msurface_t;

typedef struct
{
  int planenum;
  short children[2];
} dclipnode_t;

typedef struct
{
  dclipnode_t* clipnodes;
  mplane_t* planes;
  int firstclipnode;
  int lastclipnode;
  Vector clip_mins;
  Vector clip_maxs;
} hull_t;

typedef struct
{
  void* data;
} cache_user_t;

typedef struct
{
  char name[64];
  qboolean needload;
  modtype_t type;
  int numframes;
  synctype_t synctype;
  int flags;
  Vector mins;
  Vector maxs;
  float radius;
  int firstmodelsurface;
  int nummodelsurfaces;
  int numsubmodels;
  dmodel_t* submodels;
  int numplanes;
  mplane_t* planes;
  int numleafs;
  mleaf_t* leafs;
  int numvertexes;
  mvertex_t* vertexes;
  int numedges;
  medge_t* edges;
  int numnodes;
  mnode_t* nodes;
  int numtexinfo;
  mtexinfo_t* texinfo;
  int numsurfaces;
  msurface_t* surfaces;
  int numsurfedges;
  int* surfedges;
  int numclipnodes;
  dclipnode_t* clipnodes;
  int nummarksurfaces;
  msurface_t** marksurfaces;
  hull_t hulls[4];
  int numtextures;
  texture_t** textures;
  byte* visdata;
  color24* lightdata;
  sdk_string_const char* entities;
  cache_user_t cache;
} model_t;

typedef struct
{
  int ambientlight;
  int shadelight;
  Vector color;
  float* plightvec;
} alight_t;

typedef struct
{
  int userid;
  char userinfo[256];
  char name[32];
  int spectator;
  int ping;
  int packet_loss;
  char model[64];
  int topcolor;
  int bottomcolor;
  int renderframe;
  int gaitsequence;
  float gaitframe;
  float gaityaw;
  Vector prevgaitorigin;
  customization_t customdata;
} player_info_t;
