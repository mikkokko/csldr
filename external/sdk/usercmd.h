typedef struct
{
  short lerp_msec;
  byte msec;
  Vector viewangles;
  float forwardmove;
  float sidemove;
  float upmove;
  byte lightlevel;
  unsigned short buttons;
  byte impulse;
  byte weaponselect;
  int impact_index;
  Vector impact_position;
} usercmd_t;
