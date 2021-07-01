#include "pch.h"

engine_studio_api_t IEngineStudio;
r_studio_interface_t studio;

extern cvar_t *viewmodel_fov;

extern cvar_t *viewmodel_offset_x;
extern cvar_t *viewmodel_offset_y;
extern cvar_t *viewmodel_offset_z;

/* mikkotodo yank these from elsewhere? */

/* was 4 but fucked up my ads */
#define Z_NEAR 1.0
#define Z_FAR 4096.0

float CalcVerticalFov(float fov)
{
	/* hardcoded 4:3 aspect ratio so i don't need to do hor+ on vm fov */
	float x;

	x = 4.0f / TANF(fov / 360.0f * FL_PI);
	return ATANF(3.0f / x) * 360.0f / FL_PI;
}

static vec3_t origin_backup[128];

void ChangeModelOrigin(studiohdr_t *hdr)
{
	int i;
	mstudiobone_t *bone;

	bone = (mstudiobone_t *)((byte *)hdr + hdr->boneindex);

	for (i = 0; i < hdr->numbones; i++, bone++)
	{
		if (bone->parent == -1)
		{
			origin_backup[i][0] = bone->value[0];
			origin_backup[i][1] = bone->value[1];
			origin_backup[i][2] = bone->value[2];

			bone->value[0] += viewmodel_offset_y->value;
			bone->value[1] += viewmodel_offset_x->value;
			bone->value[2] += viewmodel_offset_z->value;
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

int Hk_StudioDrawModel(int flags)
{
	int result;
	SCREENINFO scr;
	double top, aspect;
	float fov, fov1, fov2;

	cl_entity_t *entity = IEngineStudio.GetCurrentEntity();
	bool viewModel = (entity == IEngineStudio.GetViewEntity());

	if (!viewModel)
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

	/* worse than hitler */
	ChangeModelOrigin((studiohdr_t *)entity->model->cache.data);

	result = studio.StudioDrawModel(flags);

	/* worse than hitler */
	RestoreModelOrigin((studiohdr_t *)entity->model->cache.data);

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

	if (*(uint32 *)body->name != *(uint32 *)"arms") /* it's not, bail out */
	{
		IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);
		return;
	}

	/* bodypart is right one, do cool arm changing stuff */
	oldbody = entity->curstate.body;

	if (user1 == 4)
		entity->curstate.body = playerInfo[user2].team;
	else
		entity->curstate.body = localTeam;

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
