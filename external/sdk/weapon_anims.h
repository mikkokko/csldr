typedef enum {
  GLOCK18_IDLE1 = 0,
  GLOCK18_IDLE2 = 1,
  GLOCK18_IDLE3 = 2,
  GLOCK18_SHOOT = 3,
  GLOCK18_SHOOT2 = 4,
  GLOCK18_SHOOT3 = 5,
  GLOCK18_SHOOT_EMPTY = 6,
  GLOCK18_RELOAD = 7,
  GLOCK18_DRAW = 8,
  GLOCK18_HOLSTER = 9,
  GLOCK18_ADD_SILENCER = 10,
  GLOCK18_DRAW2 = 11,
  GLOCK18_RELOAD2 = 12
} glock18_e;

typedef enum {
  USP_IDLE = 0,
  USP_SHOOT1 = 1,
  USP_SHOOT2 = 2,
  USP_SHOOT3 = 3,
  USP_SHOOT_EMPTY = 4,
  USP_RELOAD = 5,
  USP_DRAW = 6,
  USP_ATTACH_SILENCER = 7,
  USP_UNSIL_IDLE = 8,
  USP_UNSIL_SHOOT1 = 9,
  USP_UNSIL_SHOOT2 = 10,
  USP_UNSIL_SHOOT3 = 11,
  USP_UNSIL_SHOOT_EMPTY = 12,
  USP_UNSIL_RELOAD = 13,
  USP_UNSIL_DRAW = 14,
  USP_DETACH_SILENCER = 15
} usp_e;

typedef enum {
  ELITE_IDLE = 0,
  ELITE_IDLE_LEFTEMPTY = 1,
  ELITE_SHOOTLEFT1 = 2,
  ELITE_SHOOTLEFT2 = 3,
  ELITE_SHOOTLEFT3 = 4,
  ELITE_SHOOTLEFT4 = 5,
  ELITE_SHOOTLEFT5 = 6,
  ELITE_SHOOTLEFTLAST = 7,
  ELITE_SHOOTRIGHT1 = 8,
  ELITE_SHOOTRIGHT2 = 9,
  ELITE_SHOOTRIGHT3 = 10,
  ELITE_SHOOTRIGHT4 = 11,
  ELITE_SHOOTRIGHT5 = 12,
  ELITE_SHOOTRIGHTLAST = 13,
  ELITE_RELOAD = 14,
  ELITE_DRAW = 15
} elite_e;

typedef enum {
  M4A1_IDLE = 0,
  M4A1_SHOOT1 = 1,
  M4A1_SHOOT2 = 2,
  M4A1_SHOOT3 = 3,
  M4A1_RELOAD = 4,
  M4A1_DRAW = 5,
  M4A1_ATTACH_SILENCER = 6,
  M4A1_UNSIL_IDLE = 7,
  M4A1_UNSIL_SHOOT1 = 8,
  M4A1_UNSIL_SHOOT2 = 9,
  M4A1_UNSIL_SHOOT3 = 10,
  M4A1_UNSIL_RELOAD = 11,
  M4A1_UNSIL_DRAW = 12,
  M4A1_DETACH_SILENCER = 13
} m4a1_e;
