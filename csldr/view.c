#include "pch.h"

/* values for cl_bobstyle */
enum { BOB_DEFAULT, BOB_OLD, BOB_CSTRIKE15 };

cvar_t *viewmodel_fov;
cvar_t *viewmodel_shift;
cvar_t *viewmodel_offset_x;
cvar_t *viewmodel_offset_y;
cvar_t *viewmodel_offset_z;
cvar_t *viewmodel_hands;

cvar_t *cl_bobstyle;

cvar_t *cl_bobcycle;
cvar_t *cl_bobup;
cvar_t *cl_bob;

cvar_t *cl_bobamt_vert;
cvar_t *cl_bobamt_lat;
cvar_t *cl_bob_lower_amt;

cvar_t *cl_rollangle;
cvar_t *cl_rollspeed;

cvar_t *viewmodel_lag_scale;
cvar_t *viewmodel_lag_speed;

cvar_t *fov_horplus;
cvar_t *fov_lerp;

cvar_t *cl_mirror_knife;

void ViewInit(void)
{
	CVAR_ARHCIVE_FAST(viewmodel_fov, 68);
	CVAR_ARHCIVE_FAST(viewmodel_shift, 0);
	CVAR_ARHCIVE_FAST(viewmodel_offset_x, 0);
	CVAR_ARHCIVE_FAST(viewmodel_offset_y, 0);
	CVAR_ARHCIVE_FAST(viewmodel_offset_z, 0);
	CVAR_ARHCIVE_FAST(viewmodel_hands, );

	CVAR_ARHCIVE_FAST(cl_bobstyle, 0);

	cl_bobcycle = gEngfuncs.pfnGetCvarPointer("cl_bobcycle");
	cl_bobup = gEngfuncs.pfnGetCvarPointer("cl_bobup");
	cl_bob = gEngfuncs.pfnGetCvarPointer("cl_bob");

	cl_bobcycle->flags |= FCVAR_ARCHIVE;
	cl_bobup->flags |= FCVAR_ARCHIVE;

	CVAR_ARHCIVE_FAST(cl_bobamt_vert, 0.13);
	CVAR_ARHCIVE_FAST(cl_bobamt_lat, 0.32);
	CVAR_ARHCIVE_FAST(cl_bob_lower_amt, 8);

	CVAR_ARHCIVE_FAST(cl_rollangle, 2.0);
	CVAR_ARHCIVE_FAST(cl_rollspeed, 200);

	CVAR_ARHCIVE_FAST(viewmodel_lag_scale, 1.0);
	CVAR_ARHCIVE_FAST(viewmodel_lag_speed, 8.0);

	CVAR_ARHCIVE_FAST(fov_horplus, 1);
	CVAR_ARHCIVE_FAST(fov_lerp, 0.1);

	CVAR_ARHCIVE_FAST(cl_mirror_knife, 1);
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

	speed = Vec2_Length(pparams->simvel);

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
		cycle = M_PI * cycle / cl_bobup->value;
	else
		cycle = M_PI + M_PI * (cycle - cl_bobup->value) / (1.0f - cl_bobup->value);

	bobScale = 0.00625f;

	if (!pparams->onground)
		bobScale = 0.00125f;

	g_bobVars.vertBob = speed * (bobScale * cl_bobamt_vert->value);
	g_bobVars.vertBob = (g_bobVars.vertBob * 0.3f + g_bobVars.vertBob * 0.7f * sin(cycle));
	g_bobVars.vertBob = CLAMP(g_bobVars.vertBob - lowerAmt, -8, 4);

	cycle = g_bobVars.bobTime - (int)(g_bobVars.bobTime / bobCycle * 2) * bobCycle * 2;
	cycle /= bobCycle * 2;

	if (cycle < cl_bobup->value)
		cycle = M_PI * cycle / cl_bobup->value;
	else
		cycle = M_PI + M_PI * (cycle - cl_bobup->value) / (1.0f - cl_bobup->value);

	g_bobVars.horBob = speed * (bobScale * cl_bobamt_lat->value);
	g_bobVars.horBob = g_bobVars.horBob * 0.3f + g_bobVars.horBob * 0.7f * sin( cycle);
	g_bobVars.horBob = CLAMP(g_bobVars.horBob, -7, 4);
}

static void V_AddBob(ref_params_t *pparams, vec3_t origin, vec3_t angles)
{
	vec3_t forward, side;

	V_CalcBob(pparams);

	AnglesToMatrix(angles, forward, side, NULL);

	Vec3_MulAddAlt(origin, forward, g_bobVars.vertBob * 0.4f);

	origin[2] += g_bobVars.vertBob * 0.1f;

	Vec3_MulAddAlt(origin, side, g_bobVars.horBob * 0.2f);
}

static void V_AddLag(ref_params_t *pparams, vec3_t origin, vec3_t angles)
{
	vec3_t forward;
	vec3_t delta_angles;
	static vec3_t last_angles;

	AnglesToMatrix(angles, forward, NULL, NULL);

	delta_angles[0] = forward[0] - last_angles[0];
	delta_angles[1] = forward[1] - last_angles[1];
	delta_angles[2] = forward[2] - last_angles[2];

	Vec3_MulAdd(last_angles, delta_angles, viewmodel_lag_speed->value * pparams->frametime);
	Vec3_MulAddAlt(origin, delta_angles, -1 * viewmodel_lag_scale->value);
}

static void V_OffsetViewmodel(cl_entity_t *vm, vec3_t angles)
{
	vec3_t front, side, up;
	float x, y, z;

	AnglesToMatrix(angles, front, side, up);

	if (!isSoftware && currentWeapon.m_iId == WEAPON_KNIFE && cl_mirror_knife->value)
	{
		x = -viewmodel_offset_x->value;
	}
	else
		x = viewmodel_offset_x->value;

	y = viewmodel_offset_y->value;
	z = viewmodel_offset_z->value;

	Vec3_MulAdd(vm->origin, side, x);
	Vec3_MulAdd(vm->origin, front, y);
	Vec3_MulAdd(vm->origin, up, z);
}

void Hk_CalcRefdef(ref_params_t *pparams)
{
	pparams->movevars->rollangle = cl_rollangle->value;
	pparams->movevars->rollspeed = cl_rollspeed->value;
	
	if (cl_bobstyle->value == BOB_CSTRIKE15)
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

	if (!pparams->intermission && !pparams->paused)
	{
		cl_entity_t *vm;
	
		vm = gEngfuncs.GetViewModel();

		V_OffsetViewmodel(vm, pparams->viewangles);
	
		/* fuck this annoying shift */
		if (!viewmodel_shift->value)
			vm->origin[2] += 1;
	
		if (cl_bobstyle->value == BOB_CSTRIKE15)
			V_AddBob(pparams, vm->origin, vm->angles);
		else if (cl_bobstyle->value == BOB_OLD)
		{
			vm->curstate.angles[0] = vm->angles[0];
			vm->curstate.angles[1] = vm->angles[1];
			vm->curstate.angles[2] = vm->angles[2];
		}
	
		if (viewmodel_lag_scale->value)
			V_AddLag(pparams, vm->origin, vm->angles);
	
		/* mikkotodo move? */
		CameraApplyMovement(pparams);
	}

	/* mikkotodo move? */
	FovThink();
}
