typedef enum {
  t_sound = 0,
  t_skin = 1,
  t_model = 2,
  t_decal = 3,
  t_generic = 4,
  t_eventscript = 5,
  t_world = 6
} resourcetype_t;

typedef struct resource_s {
  char szFileName[64];
  resourcetype_t type;
  int nIndex;
  int nDownloadSize;
  unsigned char ucFlags;
  unsigned char rgucMD5_hash[16];
  unsigned char playernum;
  unsigned char rguc_reserved[32];
  struct resource_s* pNext;
  struct resource_s* pPrev;
} resource_t;

typedef struct customization_s {
  qboolean bInUse;
  resource_t resource;
  qboolean bTranslated;
  int nUserData1;
  int nUserData2;
  void* pInfo;
  void* pBuffer;
  struct customization_s* pNext;
} customization_t;
