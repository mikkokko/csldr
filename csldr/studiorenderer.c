#include "pch.h"

engine_studio_api_t IEngineStudio;
r_studio_interface_t studio;

extern cvar_t *viewmodel_fov;

extern cvar_t *viewmodel_offset_x;
extern cvar_t *viewmodel_offset_y;
extern cvar_t *viewmodel_offset_z;

extern cvar_t *cl_mirror_knife;

/* mikkotodo yank these from elsewhere? */

/* was 4 but fucked up my ads */
#define Z_NEAR 1.0
#define Z_FAR 4096.0

float CalcVerticalFov(float fov)
{
	/* hardcoded 4:3 aspect ratio so i don't need to do hor+ on vm fov */
	float x;

	x = 4.0f / tanf(fov / 360.0f * FL_PI);
	return atanf(3.0f / x) * 360.0f / FL_PI;
}

static vec3_t origin_backup[128];

void ChangeModelOrigin(studiohdr_t *hdr)
{
	int i;
	float x, y, z;
	mstudiobone_t *bone;

	if (currentWeapon.m_iId == WEAPON_KNIFE && cl_mirror_knife->value)
	{
		x = -viewmodel_offset_x->value;
	}
	else
		x = viewmodel_offset_x->value;

	y = viewmodel_offset_y->value;
	z = viewmodel_offset_z->value;

	bone = (mstudiobone_t *)((byte *)hdr + hdr->boneindex);

	for (i = 0; i < hdr->numbones; i++, bone++)
	{
		if (bone->parent == -1)
		{
			origin_backup[i][0] = bone->value[0];
			origin_backup[i][1] = bone->value[1];
			origin_backup[i][2] = bone->value[2];

			bone->value[0] += y;
			bone->value[1] += x;
			bone->value[2] += z;
		}
	}
}

void RestoreModelOrigin(studiohdr_t *hdr)
{
	int i;
	mstudiobone_t *bone;

	bone = (mstudiobone_t *)((byte *)hdr + hdr->boneindex);

	for (i = 0; i < hdr->numbones; i++, bone++)
	{
		if (bone->parent == -1)
		{
			bone->value[0] = origin_backup[i][0];
			bone->value[1] = origin_backup[i][1];
			bone->value[2] = origin_backup[i][2];
		}
	}
}

void UnflipKnife(float *value)
{
	if (currentWeapon.m_iId != WEAPON_KNIFE || cl_mirror_knife->value)
		return;

	*value = cl_righthand->value;
	cl_righthand->value = !cl_righthand->value;
}

void ReflipKnife(float value)
{
	if (currentWeapon.m_iId != WEAPON_KNIFE || cl_mirror_knife->value)
		return;

	cl_righthand->value = value;
}

int Hk_StudioDrawModel(int flags)
{
	int result;
	SCREENINFO scr;
	double top, aspect;
	float fov, fov1, fov2;
	float old_righthand;

	cl_entity_t *entity = IEngineStudio.GetCurrentEntity();

	if (entity != IEngineStudio.GetViewEntity())
		return studio.StudioDrawModel(flags);

	scr.iSize = sizeof(SCREENINFO);

	gEngfuncs.pfnGetScreenInfo(&scr);
	aspect = (double)scr.iWidth / (double)scr.iHeight;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	fov1 = viewmodel_fov->value * fovDifference;
	fov2 = CLAMP(fov1, 1.0f, 170.0f);
	fov = CalcVerticalFov(fov2);

	top = tan(fov * M_PI / 360.0f) * Z_NEAR;

	glFrustum(-top * aspect, top * aspect, -top, top, Z_NEAR, Z_FAR);

	glMatrixMode(GL_MODELVIEW);

	/* think about inspecting now since we're about to draw the vm */
	InspectThink();

	UnflipKnife(&old_righthand);

	/* worse than hitler */
	ChangeModelOrigin((studiohdr_t *)entity->model->cache.data);

	result = studio.StudioDrawModel(flags);

	/* worse than hitler */
	RestoreModelOrigin((studiohdr_t *)entity->model->cache.data);

	ReflipKnife(old_righthand);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	/* do camera stuff after drawing the vm */
	CameraCalcMovement(entity);

	return result;
}

void Hk_StudioSetupModel(int bodypart, void **ppbodypart, void **ppsubmodel)
{
	cl_entity_t *entity;
	bool viewModel;
	studiohdr_t *studiohdr;
	mstudiobodyparts_t *body;
	int oldbody;
	int current;
	int newbody;

	entity = IEngineStudio.GetCurrentEntity();
	viewModel = (entity == IEngineStudio.GetViewEntity());

	if (!viewModel) /* we don't want to do anything cool */
	{
		IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);
		return;
	}

	/* check if bodypart is right */
	studiohdr = (studiohdr_t *)entity->model->cache.data;
	body = (mstudiobodyparts_t *)((byte *)studiohdr + studiohdr->bodypartindex) + bodypart;

	/* check if the names matches and that there's enough submodels */
	if (*(uint32 *)body->name != *(uint32 *)"arms" || body->nummodels < 2)
	{
		IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);
		return;
	}

	/* bodypart is right one, do cool arm changing stuff */
	oldbody = entity->curstate.body;

	if (user1 == 4)
		newbody = playerInfo[user2].team;
	else
		newbody = localTeam;

	current = (entity->curstate.body / body->base) % body->nummodels;
	entity->curstate.body = (entity->curstate.body - (current * body->base) + (newbody * body->base));

	/* set the cool arm stuff */
	IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);

	/* restore state */
	entity->curstate.body = oldbody;
}

int Hk_GetStudioModelInterface(int version,
		r_studio_interface_t **ppinterface,
		engine_studio_api_t *pstudio)
{
	int result;

	/* backup and change engine sided stuff */
	memcpy(&IEngineStudio, pstudio, sizeof(engine_studio_api_t));
	pstudio->StudioSetupModel = Hk_StudioSetupModel;

	/* give to client */
	result = cl_funcs.pStudioInterface(version, ppinterface, pstudio);

	/* backup and change client sided stuff */
	memcpy(&studio, (*ppinterface), sizeof(studio));
	(*ppinterface)->StudioDrawModel = Hk_StudioDrawModel;
	
	/* if someone tries to run with software mode, tell them to get fuck */
	if (!IEngineStudio.IsHardware())
		Plat_Error("Software mode is not supported\n");

	return result;
}
