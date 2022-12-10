#include "pch.h"

playerinfo_t playerInfo[65];

int localTeam;

int user1;
int user2;

int (*Og_MsgFunc_TeamInfo)(const char *pszName, int iSize, void *pbuf);

int Hk_MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	int team;
	int index;
	const char *teamName;

	index = *(byte *)pbuf;
	teamName = (const char *)pbuf + 1;

	if (!strcmp(teamName, "CT"))
		team = TEAM_CT;
	else
		team = TEAM_TERRORIST; /* mikkotodo bad */

	playerInfo[index].team = team;

	if (index == gEngfuncs.GetLocalPlayer()->index)
		localTeam = team;

	return Og_MsgFunc_TeamInfo(pszName, iSize, pbuf);
}

void Hk_ProcessPlayerState(entity_state_t *dst, const entity_state_t *src)
{
	cl_entity_t *localPlayer = gEngfuncs.GetLocalPlayer();

	if (localPlayer->index == dst->number)
	{
		user1 = src->iuser1;
		user2 = src->iuser2;
	}

	cl_funcs.pProcessPlayerState(dst, src);
}
