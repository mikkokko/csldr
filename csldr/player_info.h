/* mikkotodo: should have 0 as no team */
#define TEAM_TERRORIST 0
#define TEAM_CT 1

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
extern int user3;

extern int (*Og_MsgFunc_TeamInfo)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf);

void Hk_ProcessPlayerState(entity_state_t *dst, const entity_state_t *src);
