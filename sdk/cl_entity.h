typedef struct efrag_s
{
	mleaf_t *leaf;
	struct efrag_s *leafnext;
	struct cl_entity_s *entity;
	struct efrag_s *entnext;
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
	byte prevseqblending[2];
	vec3_t prevorigin;
	vec3_t prevangles;
	int prevsequence;
	float prevframe;
	byte prevcontroller[4];
	byte prevblending[2];
} latchedvars_t;

typedef struct
{
	float animtime;
	vec3_t origin;
	vec3_t angles;
} position_history_t;

typedef struct cl_entity_s
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
	vec3_t origin;
	vec3_t angles;
	vec3_t attachment[4];
	int trivial_accept;
	model_t *model;
	efrag_t *efrag;
	mnode_t *topnode;
	float syncbase;
	int visframe;
	colorVec cvFloorColor;
} cl_entity_t;
