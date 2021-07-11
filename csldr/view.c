#include "pch.h"

cvar_t *viewmodel_fov;
cvar_t *viewmodel_shift;

cvar_t *viewmodel_offset_x;
cvar_t *viewmodel_offset_y;
cvar_t *viewmodel_offset_z;

cvar_t *cl_use_new_bob;

cvar_t *cl_bobcycle;
cvar_t *cl_bobup;
cvar_t *cl_bob;

cvar_t *cl_bobamt_vert;
cvar_t *cl_bobamt_lat;
cvar_t *cl_bob_lower_amt;

cvar_t *viewmodel_lag_scale;
cvar_t *viewmodel_lag_speed;

cvar_t *fov_horplus;
cvar_t *fov_lerp;

void ViewInit(void)
{
	CVAR_ARHCIVE_FAST(viewmodel_fov, 68);
	CVAR_ARHCIVE_FAST(viewmodel_shift, 0);
	CVAR_ARHCIVE_FAST(viewmodel_offset_x, 0);
	CVAR_ARHCIVE_FAST(viewmodel_offset_y, 0);
	CVAR_ARHCIVE_FAST(viewmodel_offset_z, 0);

	CVAR_ARHCIVE_FAST(cl_use_new_bob, 1);

	cl_bobcycle = gEngfuncs.pfnGetCvarPointer("cl_bobcycle");
	cl_bobup = gEngfuncs.pfnGetCvarPointer("cl_bobup");
	cl_bob = gEngfuncs.pfnGetCvarPointer("cl_bob");

	cl_bobcycle->flags |= FCVAR_ARCHIVE;
	cl_bobup->flags |= FCVAR_ARCHIVE;

	CVAR_ARHCIVE_FAST(cl_bobamt_vert, 0.13);
	CVAR_ARHCIVE_FAST(cl_bobamt_lat, 0.32);
	CVAR_ARHCIVE_FAST(cl_bob_lower_amt, 8.0);

	CVAR_ARHCIVE_FAST(viewmodel_lag_scale, 1.0);
	CVAR_ARHCIVE_FAST(viewmodel_lag_speed, 8.0);

	CVAR_ARHCIVE_FAST(fov_horplus, 1);
	CVAR_ARHCIVE_FAST(fov_lerp, 0.1);
}

struct
{
	float bobTime;
	float lastBobTime;
	float lastSpeed;
	float vertBob;
	float horBob;
} g_bobVars;

float Map(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

void V_CalcBob(ref_params_t *pparams)
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

	speed = SQRTF(
			pparams->simvel[0] * pparams->simvel[0] + pparams->simvel[1] *
			pparams->simvel[1]);

	maxSpeedDelta = MAX(0.0f,
			(pparams->time - g_bobVars.lastBobTime) * 620.0f);

	speed = CLAMP(speed,
			g_bobVars.lastSpeed - maxSpeedDelta,
			g_bobVars.lastSpeed + maxSpeedDelta);
	speed = CLAMP(speed, -320.0f, 320.0f);

	g_bobVars.lastSpeed = speed;

	lowerAmt = cl_bob_lower_amt->value * (speed * 0.001f);

	bobOffset = Map(speed, 0.0f, 320.0f, 0.0f, 1.0f);

	g_bobVars.bobTime += (pparams->time - g_bobVars.lastBobTime) * bobOffset;
	g_bobVars.lastBobTime = pparams->time;

	/* scale the bob by 1.25, this wasn't in 10040 but this way
	cs 1.6's default cl_bobcycle value (0.8) will look right */
	bobCycle = (((1000.0f - 150.0f) / 3.5f) * 0.001f) *
			   cl_bobcycle->value * 1.25f;

	cycle = g_bobVars.bobTime - (int)(g_bobVars.bobTime / bobCycle) *
			bobCycle;

	cycle /= bobCycle;

	if (cycle < cl_bobup->value)
		cycle = FL_PI * cycle / cl_bobup->value;
	else
		cycle = FL_PI + FL_PI * (cycle - cl_bobup->value) /
				(1.0f - cl_bobup->value);

	bobScale = 0.00625f;

	if (!pparams->onground)
		bobScale = 0.00125f;

	g_bobVars.vertBob = speed * (bobScale * cl_bobamt_vert->value);
	g_bobVars.vertBob =
		(g_bobVars.vertBob * 0.3f + g_bobVars.vertBob * 0.7f * SINF(cycle));
	g_bobVars.vertBob = CLAMP(g_bobVars.vertBob - lowerAmt, -8.0f, 4.0f);

	cycle = g_bobVars.bobTime - (int)(g_bobVars.bobTime / bobCycle * 2.0f) *
			bobCycle * 2.0f;
	cycle /= bobCycle * 2.0f;

	if (cycle < cl_bobup->value)
		cycle = FL_PI * cycle / cl_bobup->value;
	else
		cycle = FL_PI + FL_PI * (cycle - cl_bobup->value) /
				(1.0f - cl_bobup->value);

	g_bobVars.horBob = speed * (bobScale * cl_bobamt_lat->value);
	g_bobVars.horBob = g_bobVars.horBob * 0.3f + g_bobVars.horBob * 0.7f * SINF(
			cycle);
	g_bobVars.horBob = CLAMP(g_bobVars.horBob, -7.0f, 4.0f);
}

void V_AddBob(ref_params_t *pparams, vec3_t origin, vec3_t angles)
{
	vec3_t forward, right;

	V_CalcBob(pparams);

	AngleVectors(angles, forward, right, NULL);

	InverseRollVectorMA(origin, g_bobVars.vertBob * 0.4f, forward, origin);

	origin[2] += g_bobVars.vertBob * 0.1f;

	InverseRollVectorMA(origin, g_bobVars.horBob * 0.2f, right, origin);
}

void V_AddLag(ref_params_t *pparams, vec3_t origin, vec3_t angles)
{
	vec3_t forward;
	vec3_t delta_angles;
	static vec3_t last_angles;

	AngleVectors(angles, forward, NULL, NULL);

	delta_angles[0] = forward[0] - last_angles[0];
	delta_angles[1] = forward[1] - last_angles[1];
	delta_angles[2] = forward[2] - last_angles[2];

	VectorMA(last_angles, viewmodel_lag_speed->value * pparams->frametime, delta_angles, last_angles);
	InverseRollVectorMA(origin, -1.0f * viewmodel_lag_scale->value, delta_angles, origin);
}

void Hk_CalcRefdef(ref_params_t *pparams)
{
	if (cl_use_new_bob->value)
	{
		float bobcycle, bobup, bob;

		bobcycle = cl_bobcycle->value;
		bobup = cl_bobup->value;
		bob = cl_bob->value;

		cl_bobcycle->value = 0.0f;
		cl_bobup->value = 0.0f;
		cl_bob->value = 0.0f;

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

		/* fuck this annoying shift */
		if (!viewmodel_shift->value)
			vm->origin[2] += 1;

		if (cl_use_new_bob->value)
			V_AddBob(pparams, vm->origin, vm->angles);
		
		if (viewmodel_lag_scale->value)
			V_AddLag(pparams, vm->origin, vm->angles);

		/* mikkotodo move? */
		CameraApplyMovement(pparams);
	}

	/* mikkotodo move? */
	FovThink();
}
