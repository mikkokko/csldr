#include "pch.h"

cvar_t *mirror_shell;
cvar_t *cl_righthand;

bool recalcShell;
float shellRotation;
vec3_t shellOrigin;
vec3_t shellVelocity;

void ShellInit(void)
{
	cl_righthand = gEngfuncs.pfnGetCvarPointer("cl_righthand");
	mirror_shell = gEngfuncs.pfnRegisterVariable("mirror_shell", "1", FCVAR_ARCHIVE);
}

int (*Og_MsgFunc_Brass)(const char *pszName, int iSize, void *pbuf);

int Hk_MsgFunc_Brass(const char *pszName, int iSize, void *pbuf)
{
	if (!isSoftware && mirror_shell->value)
	{
		/* save origin and velocity, we'll flip them later in R_TempModel */
		BEGIN_READ(pbuf, iSize);

		if (isCzero)
		{
			shellOrigin[0] = READ_COORD();
			shellOrigin[1] = READ_COORD();
			shellOrigin[2] = READ_COORD();
			shellVelocity[0] = READ_COORD();
			shellVelocity[1] = READ_COORD();
			shellVelocity[2] = READ_COORD();
			shellRotation = READ_ANGLE();
		}
		else
		{
			READ_BYTE(); /* stupid */
			shellOrigin[0] = READ_COORD();
			shellOrigin[1] = READ_COORD();
			shellOrigin[2] = READ_COORD();
			READ_COORD(); /* stupid */
			READ_COORD(); /* stupid */
			READ_COORD(); /* stupid */
			shellVelocity[0] = READ_COORD();
			shellVelocity[1] = READ_COORD();
			shellVelocity[2] = READ_COORD();
			shellRotation = READ_ANGLE();
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
		float sini, kosini;
		float x, y;

		recalcShell = false;

		rot = RAD(shellRotation);

		sini = sin(rot);
		kosini = cos(rot);

		if (!cl_righthand->value)
		{
			shellVelocity[0] = shellVelocity[0] - sini * 120.0f;
			shellVelocity[1] = kosini * 120.0f + shellVelocity[1];

			x = -9.0f * sini;
			y = 9.0f * kosini;
		}
		else
		{
			x = 9.0f * sini;
			y = -9.0f * kosini;
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

/* i almost did not include shell mirroring because of this */
#define FLIP_SHELL_ON_EVENT(name) \
	void(*Og_ ## name)(event_args_t * args); \
	void Hk_ ## name(event_args_t * args) \
	{ \
		float value; \
		if (isSoftware || !mirror_shell->value) \
		{ \
			Og_ ## name(args); \
			return; \
		} \
		value = cl_righthand->value; \
		cl_righthand->value = (float)(!(int)cl_righthand->value); \
		Og_ ## name(args); \
		cl_righthand->value = value; \
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
FLIP_SHELL_ON_EVENT(FireGlock1)
FLIP_SHELL_ON_EVENT(FireGlock2)
FLIP_SHELL_ON_EVENT(FireGlock18)
FLIP_SHELL_ON_EVENT(FireM249)
FLIP_SHELL_ON_EVENT(FireM4A1)
FLIP_SHELL_ON_EVENT(FireMP5)
FLIP_SHELL_ON_EVENT(FireMP5N)
FLIP_SHELL_ON_EVENT(FireMac10)
FLIP_SHELL_ON_EVENT(FireP228)
FLIP_SHELL_ON_EVENT(FireP90)
FLIP_SHELL_ON_EVENT(FireSG550)
FLIP_SHELL_ON_EVENT(FireSG552)
FLIP_SHELL_ON_EVENT(FireShotGunSingle)
FLIP_SHELL_ON_EVENT(FireShotGunDouble)
FLIP_SHELL_ON_EVENT(FireTMP)
FLIP_SHELL_ON_EVENT(FireUMP45)
FLIP_SHELL_ON_EVENT(FireUSP)
FLIP_SHELL_ON_EVENT(FireXM1014)

#define CHECK_EVENT(name, func) \
	if (!strcmp(partialName, name)) \
	{ \
		Og_ ## func = pfnEvent; \
		return Hk_ ## func; \
	} \

typedef void (*pfnEvent_t)(event_args_t *);

pfnEvent_t CheckForShellEvent(const char *name, pfnEvent_t pfnEvent)
{
	const char *partialName;

	if (memcmp(name, "events/", 7))
		return NULL;

	/* skip "events/" for comparison */
	partialName = name + 7;

	CHECK_EVENT("ak47.sc", FireAK47);
	/* CHECK_EVENT("aug.sc", FireAug); already mirrored? */
	CHECK_EVENT("deagle.sc", FireDeagle);
	CHECK_EVENT("elite_left.sc", FireEliteLeft);
	CHECK_EVENT("elite_right.sc", FireEliteRight);
	/* CHECK_EVENT("famas.sc", FireFamas); already mirrored? */
	CHECK_EVENT("fiveseven.sc", FireFiveSeven);
	CHECK_EVENT("g3sg1.sc", FireG3SG1);
	CHECK_EVENT("galil.sc", FireGalil);
	CHECK_EVENT("glock1.sc", FireGlock1);
	CHECK_EVENT("glock2.sc", FireGlock2);
	CHECK_EVENT("glock18.sc", FireGlock18);
	/* CHECK_EVENT("m249.sc", FireM249); already mirrored? */
	CHECK_EVENT("m4a1.sc", FireM4A1);
	CHECK_EVENT("mp5.sc", FireMP5);
	CHECK_EVENT("mp5n.sc", FireMP5N);
	CHECK_EVENT("mac10.sc", FireMac10);
	CHECK_EVENT("p228.sc", FireP228);
	/* CHECK_EVENT("p90.sc", FireP90); already mirrored? */
	CHECK_EVENT("sg550.sc", FireSG550);
	CHECK_EVENT("sg552.sc", FireSG552);
	CHECK_EVENT("shotgun1.sc", FireShotGunSingle);
	CHECK_EVENT("shotgun2.sc", FireShotGunDouble);
	CHECK_EVENT("tmp.sc", FireTMP);
	CHECK_EVENT("ump45.sc", FireUMP45);
	CHECK_EVENT("usp.sc", FireUSP);
	CHECK_EVENT("xm1014.sc", FireXM1014);

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
