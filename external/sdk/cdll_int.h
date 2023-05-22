typedef int _HSPRITE;

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
  Vector origin;
  Vector viewangles;
  int iWeaponBits;
  float fov;
} client_data_t;

typedef struct
{
  char szName[64];
  char szSprite[64];
  int hspr;
  int iRes;
  wrect_t rc;
} client_sprite_t;

typedef struct
{
  sdk_string_const char* name;
  short ping;
  byte thisplayer;
  byte spectator;
  byte packetloss;
  sdk_string_const char* model;
  short topcolor;
  short bottomcolor;
  uint64 m_nSteamID;
} hud_player_info_s;
