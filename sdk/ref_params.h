UNIMPLEMENTED_TYPE(usercmd_t);
UNIMPLEMENTED_TYPE(movevars_t);

typedef struct
{
	float vieworg[3];
	float viewangles[3];
	float forward[3];
	float right[3];
	float up[3];
	float frametime;
	float time;
	int intermission;
	int paused;
	int spectator;
	int onground;
	int waterlevel;
	float simvel[3];
	float simorg[3];
	float viewheight[3];
	float idealpitch;
	float cl_viewangles[3];
	int health;
	float crosshairangle[3];
	float viewsize;
	float punchangle[3];
	int maxclients;
	int viewentity;
	int playernum;
	int max_entities;
	int demoplayback;
	int hardware;
	int smoothing;
	usercmd_t *cmd;
	movevars_t *movevars;
	int viewport[4];
	int nextView;
	int onlyClientDraw;
} ref_params_t;
