typedef struct efrag_s {
  mleaf_t* leaf;
  struct efrag_s* leafnext;
  struct cl_entity_s* entity;
  struct efrag_s* entnext;
} efrag_t;

typedef struct
{
  byte mouthopen;
  byte sndcount;
  int sndavg;
} mouth_t;

typedef struct
{
  float prevanimtime;
  float sequencetime;
  unsigned char prevseqblending[2];
  Vector prevorigin;
  Vector prevangles;
  int prevsequence;
  float prevframe;
  unsigned char prevcontroller[4];
  unsigned char prevblending[2];
} latchedvars_t;

typedef struct
{
  float animtime;
  Vector origin;
  Vector angles;
} position_history_t;

typedef struct
{
  int index;
  qboolean player;
  entity_state_t baseline;
  entity_state_t prevstate;
  entity_state_t curstate;
  int current_position;
  position_history_t ph[64];
  mouth_t mouth;
  latchedvars_t latched;
  float lastmove;
  Vector origin;
  Vector angles;
  Vector attachment[4];
  int trivial_accept;
  model_t* model;
  efrag_t* efrag;
  mnode_t* topnode;
  float syncbase;
  int visframe;
  colorVec cvFloorColor;
} cl_entity_t;
