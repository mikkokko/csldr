#include "pch.h"

cvar_t *cl_righthand;
static cvar_t *spec_pip;

static bool recalcShell;
static float shellRotation;
static vec3_t shellOrigin;
static vec3_t shellVelocity;

void ShellInit(void)
{
	cl_righthand = gEngfuncs.pfnGetCvarPointer("cl_righthand");
	spec_pip = gEngfuncs.pfnGetCvarPointer("spec_pip");
}

int (*Og_MsgFunc_Brass)(const char *pszName, int iSize, void *pbuf);

static bool ShouldMirrorShell(void)
{
	cl_entity_t *vm = gEngfuncs.GetViewModel();
	model_t *model = vm->model;
	if (!model)
		return false;

	studiohdr_t *header = (studiohdr_t *)model->cache.data;
	if (!header)
		return false;

	studio_cache_t *cache = GetStudioCache(model, header);

	return cache->mirror_shell;
}

int Hk_MsgFunc_Brass(const char *pszName, int iSize, void *pbuf)
{
	if (ShouldMirrorShell())
	{
		msg_read_t read;

		/* save origin and velocity, we'll flip them later in R_TempModel */
		Msg_ReadInit(&read, pbuf, iSize);

		if (isCzero)
		{
			shellOrigin[0] = Msg_ReadCoord(&read);
			shellOrigin[1] = Msg_ReadCoord(&read);
			shellOrigin[2] = Msg_ReadCoord(&read);
			shellVelocity[0] = Msg_ReadCoord(&read);
			shellVelocity[1] = Msg_ReadCoord(&read);
			shellVelocity[2] = Msg_ReadCoord(&read);
			shellRotation = Msg_ReadAngle(&read);
		}
		else
		{
			Msg_ReadByte(&read); /* stupid */
			shellOrigin[0] = Msg_ReadCoord(&read);
			shellOrigin[1] = Msg_ReadCoord(&read);
			shellOrigin[2] = Msg_ReadCoord(&read);
			Msg_ReadCoord(&read); /* stupid */
			Msg_ReadCoord(&read); /* stupid */
			Msg_ReadCoord(&read); /* stupid */
			shellVelocity[0] = Msg_ReadCoord(&read);
			shellVelocity[1] = Msg_ReadCoord(&read);
			shellVelocity[2] = Msg_ReadCoord(&read);
			shellRotation = Msg_ReadAngle(&read);
		}

		recalcShell = true;
	}

	return Og_MsgFunc_Brass(pszName, iSize, pbuf);
}

TEMPENTITY *(*Og_TempModel)(float *pos, float *dir, float *angles, float life, int modelIndex, int soundtype);

TEMPENTITY *Hk_TempModel(float *pos,
		float *dir,
		float *angles,
		float life,
		int modelIndex,
		int soundtype)
{
	if (recalcShell)
	{
		float rot;
		float sine, cosine;
		float x, y;

		recalcShell = false;

		rot = Radians(shellRotation);

		sine = sinf(rot);
		cosine = cosf(rot);

		if (!cl_righthand->value)
		{
			shellVelocity[0] = shellVelocity[0] - sine * 120;
			shellVelocity[1] = cosine * 120 + shellVelocity[1];

			x = -9 * sine;
			y = 9 * cosine;
		}
		else
		{
			x = 9 * sine;
			y = -9 * cosine;
		}

		shellOrigin[0] += x;
		shellOrigin[1] += y;

		return Og_TempModel(shellOrigin,
				shellVelocity,
				angles,
				life,
				modelIndex,
				soundtype);
	}

	return Og_TempModel(pos, dir, angles, life, modelIndex, soundtype);
}

static bool IsLocal(int idx)
{
	if (user1 == 4 || (user1 && spec_pip->value == 2))
		return (user2 == idx);

	return (gEngfuncs.pEventAPI->EV_IsLocal(idx - 1) != 0);
}

/* i almost did not include shell mirroring because of this */
#define FLIP_SHELL_ON_EVENT(name) \
	static void(*Og_ ## name)(event_args_t * args); \
	static void Hk_ ## name(event_args_t * args) \
	{ \
		xhairShotsFired += IsLocal(args->entindex); \
		float value; \
		if (!ShouldMirrorShell()) \
		{ \
			Og_ ## name(args); \
			return; \
		} \
		value = cl_righthand->value; \
		cl_righthand->value = (float)(!(int)cl_righthand->value); \
		Og_ ## name(args); \
		cl_righthand->value = value; \
	}

#define NO_FLIP_SHELL_ON_EVENT(name) \
	static void(*Og_ ## name)(event_args_t * args); \
	static void Hk_ ## name(event_args_t * args) \
	{ \
		xhairShotsFired += IsLocal(args->entindex); \
		Og_ ## name(args); \
	}

FLIP_SHELL_ON_EVENT(FireAK47)
FLIP_SHELL_ON_EVENT(FireAug)
FLIP_SHELL_ON_EVENT(FireDeagle)
FLIP_SHELL_ON_EVENT(FireEliteLeft)
FLIP_SHELL_ON_EVENT(FireEliteRight)
FLIP_SHELL_ON_EVENT(FireFamas)
FLIP_SHELL_ON_EVENT(FireFiveSeven)
FLIP_SHELL_ON_EVENT(FireG3SG1)
FLIP_SHELL_ON_EVENT(FireGalil)
//FLIP_SHELL_ON_EVENT(FireGlock1)
//FLIP_SHELL_ON_EVENT(FireGlock2)
FLIP_SHELL_ON_EVENT(FireGlock18)
FLIP_SHELL_ON_EVENT(FireM249)
FLIP_SHELL_ON_EVENT(FireM4A1)
//FLIP_SHELL_ON_EVENT(FireMP5)
FLIP_SHELL_ON_EVENT(FireMP5N)
FLIP_SHELL_ON_EVENT(FireMac10)
FLIP_SHELL_ON_EVENT(FireP228)
FLIP_SHELL_ON_EVENT(FireP90)
FLIP_SHELL_ON_EVENT(FireSG550)
FLIP_SHELL_ON_EVENT(FireSG552)
//FLIP_SHELL_ON_EVENT(FireShotGunSingle)
//FLIP_SHELL_ON_EVENT(FireShotGunDouble)
FLIP_SHELL_ON_EVENT(FireTMP)
FLIP_SHELL_ON_EVENT(FireUMP45)
FLIP_SHELL_ON_EVENT(FireUSP)
FLIP_SHELL_ON_EVENT(FireXM1014)

NO_FLIP_SHELL_ON_EVENT(FireAWP)
//NO_FLIP_SHELL_ON_EVENT(FireGauss)
NO_FLIP_SHELL_ON_EVENT(FireM3)
//NO_FLIP_SHELL_ON_EVENT(FirePython)
NO_FLIP_SHELL_ON_EVENT(FireScout)

#define CHECK_EVENT(name, func) \
	if (!strcmp(partialName, name)) \
	{ \
		Og_ ## func = pfnEvent; \
		return Hk_ ## func; \
	} \

typedef void (*pfnEvent_t)(event_args_t *);

static pfnEvent_t CheckForShellEvent(const char *name, pfnEvent_t pfnEvent)
{
	const char *partialName;

	if (memcmp(name, "events/", 7))
		return NULL;

	/* skip "events/" for comparison */
	partialName = name + 7;

	CHECK_EVENT("ak47.sc", FireAK47)
	CHECK_EVENT("aug.sc", FireAug)
	CHECK_EVENT("awp.sc", FireAWP)
	CHECK_EVENT("deagle.sc", FireDeagle)
	CHECK_EVENT("elite_left.sc", FireEliteLeft)
	CHECK_EVENT("elite_right.sc", FireEliteRight)
	CHECK_EVENT("famas.sc", FireFamas)
	CHECK_EVENT("fiveseven.sc", FireFiveSeven)
	CHECK_EVENT("g3sg1.sc", FireG3SG1)
	CHECK_EVENT("galil.sc", FireGalil)
	//CHECK_EVENT("gauss.sc", FireGauss)
	//CHECK_EVENT("glock1.sc", FireGlock1)
	CHECK_EVENT("glock18.sc", FireGlock18)
	//CHECK_EVENT("glock2.sc", FireGlock2)
	CHECK_EVENT("m249.sc", FireM249)
	CHECK_EVENT("m3.sc", FireM3)
	CHECK_EVENT("m4a1.sc", FireM4A1)
	CHECK_EVENT("mac10.sc", FireMac10)
	//CHECK_EVENT("mp5.sc", FireMP5)
	CHECK_EVENT("mp5n.sc", FireMP5N)
	CHECK_EVENT("p228.sc", FireP228)
	CHECK_EVENT("p90.sc", FireP90)
	//CHECK_EVENT("python.sc", FirePython)
	CHECK_EVENT("scout.sc", FireScout)
	CHECK_EVENT("sg550.sc", FireSG550)
	CHECK_EVENT("sg552.sc", FireSG552)
	//CHECK_EVENT("shotgun1.sc", FireShotGunSingle)
	//CHECK_EVENT("shotgun2.sc", FireShotGunDouble)
	CHECK_EVENT("tmp.sc", FireTMP)
	CHECK_EVENT("ump45.sc", FireUMP45)
	CHECK_EVENT("usp.sc", FireUSP)
	CHECK_EVENT("xm1014.sc", FireXM1014)

	return NULL;
}

void Hk_HookEvent(const char *name, void (*pfnEvent)(event_args_t *))
{
	pfnEvent_t hooked = CheckForShellEvent(name, pfnEvent);
	if (hooked)
	{
		gEngfuncs.pfnHookEvent(name, hooked);
		return;
	}

	gEngfuncs.pfnHookEvent(name, pfnEvent);
}
