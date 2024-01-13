#include "pch.h"

vec3_t v_vieworg;
vec3_t v_viewforward, v_viewright, v_viewup;

cvar_t *viewmodel_fov;
static cvar_t *viewmodel_shift;
static cvar_t *viewmodel_offset_x;
static cvar_t *viewmodel_offset_y;
static cvar_t *viewmodel_offset_z;
cvar_t *viewmodel_hands;

static cvar_t *cl_bobstyle;

static cvar_t *cl_bobcycle;
static cvar_t *cl_bobup;
static cvar_t *cl_bob;

static cvar_t *cl_bobamt_vert;
static cvar_t *cl_bobamt_lat;
static cvar_t *cl_bob_lower_amt;

static cvar_t *cl_rollangle;
static cvar_t *cl_rollspeed;

static cvar_t *viewmodel_lag_style;
static cvar_t *viewmodel_lag_scale;
static cvar_t *viewmodel_lag_speed;

cvar_t *fov_horplus;
cvar_t *fov_lerp;

void ViewInit(void)
{
	CVAR_ARCHIVE_FAST(viewmodel_fov, 90);
	CVAR_ARCHIVE_FAST(viewmodel_shift, 0);
	CVAR_ARCHIVE_FAST(viewmodel_offset_x, 0);
	CVAR_ARCHIVE_FAST(viewmodel_offset_y, 0);
	CVAR_ARCHIVE_FAST(viewmodel_offset_z, 0);
	CVAR_ARCHIVE_FAST_STR(viewmodel_hands, "");

	CVAR_ARCHIVE_FAST(cl_bobstyle, 0);

	cl_bobcycle = gEngfuncs.pfnGetCvarPointer("cl_bobcycle");
	cl_bobup = gEngfuncs.pfnGetCvarPointer("cl_bobup");
	cl_bob = gEngfuncs.pfnGetCvarPointer("cl_bob");

	cl_bobcycle->flags |= FCVAR_ARCHIVE;
	cl_bobup->flags |= FCVAR_ARCHIVE;

	CVAR_ARCHIVE_FAST(cl_bobamt_vert, 0.13);
	CVAR_ARCHIVE_FAST(cl_bobamt_lat, 0.32);
	CVAR_ARCHIVE_FAST(cl_bob_lower_amt, 8);

	CVAR_ARCHIVE_FAST(cl_rollangle, 2.0);
	CVAR_ARCHIVE_FAST(cl_rollspeed, 200);

	CVAR_ARCHIVE_FAST(viewmodel_lag_style, 1);
	CVAR_ARCHIVE_FAST(viewmodel_lag_scale, 1.0);
	CVAR_ARCHIVE_FAST(viewmodel_lag_speed, 8.0);

	// hl 25th anniversary update added gl_widescreen_yfov which is essentially
	// the same thing, keep fov_horplus around for older clients though
	if (!gEngfuncs.pfnGetCvarPointer("gl_widescreen_yfov"))
		CVAR_ARCHIVE_FAST(fov_horplus, 1);

	CVAR_ARCHIVE_FAST(fov_lerp, 0.1);
}

struct
{
	float bobTime;
	float lastBobTime;
	float lastSpeed;
	float vertBob;
	float horBob;
} g_bobVars;

static float Map(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

static void V_CalcBob(ref_params_t *pparams)
{
	float speed;
	float maxSpeedDelta;
	float lowerAmt;
	float bobOffset;
	float bobCycle;
	float bobScale;
	float cycle;

	if ((!pparams->frametime))
		return;

	speed = VectorLength2D(pparams->simvel);

	maxSpeedDelta = MAX(0, (pparams->time - g_bobVars.lastBobTime) * 620);

	speed = CLAMP(speed, g_bobVars.lastSpeed - maxSpeedDelta, g_bobVars.lastSpeed + maxSpeedDelta);
	speed = CLAMP(speed, -320, 320);

	g_bobVars.lastSpeed = speed;

	lowerAmt = cl_bob_lower_amt->value * (speed * 0.001f);

	bobOffset = Map(speed, 0, 320, 0, 1);

	g_bobVars.bobTime += (pparams->time - g_bobVars.lastBobTime) * bobOffset;
	g_bobVars.lastBobTime = pparams->time;

	/* scale the bob by 1.25, this wasn't in 10040 but this way
	cs 1.6's default cl_bobcycle value (0.8) will look right */
	bobCycle = (((1000.0f - 150.0f) / 3.5f) * 0.001f) * cl_bobcycle->value * 1.25f;

	cycle = g_bobVars.bobTime - (int)(g_bobVars.bobTime / bobCycle) * bobCycle;
	cycle /= bobCycle;

	if (cycle < cl_bobup->value)
		cycle = F_PI * cycle / cl_bobup->value;
	else
		cycle = F_PI + F_PI * (cycle - cl_bobup->value) / (1.0f - cl_bobup->value);

	bobScale = 0.00625f;

	if (!pparams->onground)
		bobScale = 0.00125f;

	g_bobVars.vertBob = speed * (bobScale * cl_bobamt_vert->value);
	g_bobVars.vertBob = (g_bobVars.vertBob * 0.3f + g_bobVars.vertBob * 0.7f * sinf(cycle));
	g_bobVars.vertBob = CLAMP(g_bobVars.vertBob - lowerAmt, -8, 4);

	cycle = g_bobVars.bobTime - (int)(g_bobVars.bobTime / bobCycle * 2) * bobCycle * 2;
	cycle /= bobCycle * 2;

	if (cycle < cl_bobup->value)
		cycle = F_PI * cycle / cl_bobup->value;
	else
		cycle = F_PI + F_PI * (cycle - cl_bobup->value) / (1.0f - cl_bobup->value);

	g_bobVars.horBob = speed * (bobScale * cl_bobamt_lat->value);
	g_bobVars.horBob = g_bobVars.horBob * 0.3f + g_bobVars.horBob * 0.7f * sinf(cycle);
	g_bobVars.horBob = CLAMP(g_bobVars.horBob, -7, 4);
}

static void V_AddBob(ref_params_t *pparams, vec3_t origin, vec3_t front, vec3_t side)
{
	V_CalcBob(pparams);

	VectorMA_2(front, g_bobVars.vertBob * 0.4f, origin);

	origin[2] += g_bobVars.vertBob * 0.1f;

	VectorMA_2(side, g_bobVars.horBob * 0.2f, origin);
}

static void V_AddLag_HL2(ref_params_t *pparams, vec3_t origin, vec3_t front)
{
	vec3_t delta_front;
	static vec3_t last_front;

	delta_front[0] = front[0] - last_front[0];
	delta_front[1] = front[1] - last_front[1];
	delta_front[2] = front[2] - last_front[2];

	VectorMA(delta_front, viewmodel_lag_speed->value * pparams->frametime, last_front);
	VectorMA_2(delta_front, -1 * viewmodel_lag_scale->value, origin);
}

// quite an inefficent way to do css style lag but oh well
typedef struct
{
	bool valid;
	vec3_t value;
} lag_angles_t;

#define STEP_MILLIS 10
#define ANGLE_BACKUP 128

#define TO_STEP(x) ((int)((x) * STEP_MILLIS))
#define FROM_STEP(x) ((float)(x) * (1.0f / STEP_MILLIS))

static lag_angles_t lag_angles[ANGLE_BACKUP];
static int last_step;

static void AddLagAngles(float time, vec3_t angles)
{
	int step = TO_STEP(time);

	if (step < last_step)
	{
		// client restart
		memset(lag_angles, 0, sizeof(lag_angles));
	}
	else if (step == last_step)
	{
		// too soon
		return;
	}

	last_step = step;

	VectorCopy(angles, lag_angles[step % 128].value);
	lag_angles[step % ANGLE_BACKUP].valid = true;
}

static bool GetLagAngles(float time, vec3_t dest)
{
	int step = TO_STEP(time);

	lag_angles_t *angles = &lag_angles[step % ANGLE_BACKUP];
	if (!angles->valid)
		return false;

	float step_time = FROM_STEP(TO_STEP(time));
	if (step_time < time)
	{
		lag_angles_t *next_angles = &lag_angles[(step + 1) % ANGLE_BACKUP];
		if (!next_angles->valid)
		{
			// can't lerp to these
			VectorCopy(angles->value, dest);
		}
		else
		{
			float next_time = FROM_STEP(step + 1);
			float frac = Map(time, step_time, next_time, 0, 1);
			AngleLerp(angles->value, next_angles->value, frac, dest);
		}
	}
	else
	{
		// if round_time > time, the difference is
		// probably so small that it can't be noticed
		VectorCopy(angles->value, dest);
	}

	return true;
}

static void V_AddLag_CSS(ref_params_t *pparams, vec3_t origin, vec3_t angles, vec3_t front, vec3_t side, vec3_t up)
{
	AddLagAngles(pparams->time, angles);

	vec3_t prev_angles;
	if (!GetLagAngles(pparams->time - 0.1f, prev_angles))
		return;

	vec3_t delta_angles;
	VectorSubtract(prev_angles, angles, delta_angles);
	VectorNegate(delta_angles);

	vec3_t delta_front;
	AngleVectors(delta_angles, delta_front, NULL, NULL);

	VectorMA_2(front, (1 - delta_front[0]) * viewmodel_lag_scale->value, origin);
	VectorMA_2(side, (delta_front[1]) * viewmodel_lag_scale->value, origin);
	VectorMA_2(up, (-delta_front[2]) * viewmodel_lag_scale->value, origin);
}

static void V_OffsetViewmodel(cl_entity_t *vm, vec3_t front, vec3_t side, vec3_t up)
{
	vec3_t extra_origin;
	VectorClear(extra_origin);

	studio_cache_t *cache = EntityStudioCache(vm);
	if (cache)
	{
		VectorCopy(cache->config.origin, extra_origin);
	}

	float x = viewmodel_offset_x->value + extra_origin[0];
	float y = viewmodel_offset_y->value + extra_origin[1];
	float z = -(viewmodel_offset_z->value + extra_origin[2]);

	if (!cl_righthand->value)
		x = -x;

	if (ShouldMirrorViewmodel(vm))
		x = -x;

	VectorMA_2(side, x, vm->origin);
	VectorMA_2(front, y, vm->origin);
	VectorMA_2(up, z, vm->origin);
}

static void CalcCustomRefdef(ref_params_t *pparams)
{
	cl_entity_t *vm;
	vec3_t front, side, up;

	vm = gEngfuncs.GetViewModel();
	AngleVectors(vm->angles, front, side, up);

	/* fix the slight difference between view and vm origin */
	vm->origin[0] += 1.0f / 32;
	vm->origin[1] += 1.0f / 32;
	vm->origin[2] += 1.0f / 32;

	V_OffsetViewmodel(vm, front, side, up);

	/* fuck this annoying shift */
	if ((int)viewmodel_shift->value == 1)
	{
		vm->origin[2] += 1;
	}
	else if ((int)viewmodel_shift->value == 2)
	{
		// remove shift from origin
		vm->origin[2] += 1;

		// offset the viewmodel
		VectorMA_2(up, 1, vm->origin);
	}

	if ((int)cl_bobstyle->value == 2)
	{
		V_AddBob(pparams, vm->origin, front, side);
	}
	else if ((int)cl_bobstyle->value == 1)
	{
		vm->curstate.angles[0] = vm->angles[0];
		vm->curstate.angles[1] = vm->angles[1];
		vm->curstate.angles[2] = vm->angles[2];
	}
	else if ((int)cl_bobstyle->value == 0)
	{
		VectorCopy(pparams->viewangles, vm->curstate.angles);
		vm->curstate.angles[0] = -vm->curstate.angles[0];
	}

	switch ((int)viewmodel_lag_style->value)
	{
	case 1:
		V_AddLag_HL2(pparams, vm->origin, front);
		break;

	case 2:
		V_AddLag_CSS(pparams, vm->origin, vm->angles, front, side, up);
		break;
	}

	/* mikkotodo move? */
	CameraApplyMovement(pparams);
}

void Hk_CalcRefdef(ref_params_t *pparams)
{
	float old_rollangle, old_rollspeed;

	/* temporarily override view roll settings with client dictated ones */
	old_rollangle = pparams->movevars->rollangle;
	old_rollspeed = pparams->movevars->rollspeed;
	pparams->movevars->rollangle = cl_rollangle->value;
	pparams->movevars->rollspeed = cl_rollspeed->value;

	if ((int)cl_bobstyle->value == 2)
	{
		float bobcycle, bobup, bob;

		bobcycle = cl_bobcycle->value;
		bobup = cl_bobup->value;
		bob = cl_bob->value;

		cl_bobcycle->value = 0;
		cl_bobup->value = 0;
		cl_bob->value = 0;

		cl_funcs.pCalcRefdef(pparams);

		cl_bobcycle->value = bobcycle;
		cl_bobup->value = bobup;
		cl_bob->value = bob;
	}
	else
	{
		cl_funcs.pCalcRefdef(pparams);
	}

	/* view roll has been applied, restore the settings */
	pparams->movevars->rollangle = old_rollangle;
	pparams->movevars->rollspeed = old_rollspeed;

	if (!pparams->intermission && !pparams->paused)
	{
		CalcCustomRefdef(pparams);
	}

	/* mikkotodo move? */
	FovThink();

	VectorCopy(pparams->vieworg, v_vieworg);
	VectorCopy(pparams->forward, v_viewforward);
	VectorCopy(pparams->right, v_viewright);
	VectorCopy(pparams->up, v_viewup);

	// dumb as fuck
	R_StudioNewFrame();
}
