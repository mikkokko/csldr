enum
{
	TEAM_UNASSIGNED = 0,
	TEAM_TERRORIST = 1,
	TEAM_CT = 2,
	TEAM_SPECTATOR = 3
};

/* custom player info struct */
typedef struct
{
	int team;
} playerinfo_t;

extern playerinfo_t playerInfo[65];

/* local player info */
extern int localTeam;

extern int user1;
extern int user2;

extern int (*Og_MsgFunc_TeamInfo)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf);

void Hk_ProcessPlayerState(entity_state_t *dst, const entity_state_t *src);
