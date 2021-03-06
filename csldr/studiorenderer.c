#include "pch.h"

bool isSoftware;
bool isOpenGL;

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

	x = 4.0f / tan(RADIANS(fov) / 2);
	return DEGREES(atan(3.0f / x)) * 2;
}

void UnflipKnife(float *value)
{
	if (isSoftware)
		return;

	if (currentWeapon.m_iId != WEAPON_KNIFE || cl_mirror_knife->value)
		return;

	*value = cl_righthand->value;
	cl_righthand->value = (float)!cl_righthand->value;
}

void ReflipKnife(float value)
{
	if (isSoftware)
		return;

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

	/* viewmodel fov only supported on opengl */
	if (isOpenGL)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		fov1 = viewmodel_fov->value * fovDifference;
		fov2 = CLAMP(fov1, 1, 170);
		fov = CalcVerticalFov(fov2);

		top = tan(RADIANS(fov) / 2) * Z_NEAR;

		glFrustum(-top * aspect, top * aspect, -top, top, Z_NEAR, Z_FAR);

		glMatrixMode(GL_MODELVIEW);
	}

	/* think about inspecting now since we're about to draw the vm */
	InspectThink();

	UnflipKnife(&old_righthand);

	result = studio.StudioDrawModel(flags);

	ReflipKnife(old_righthand);

	/* viewmodel fov only supported on opengl */
	if (isOpenGL)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

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
	if (!studiohdr) /* what the fuck */
	{
		IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);
		return;
	}

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

	/* backup and change engine stuff */
	memcpy(&IEngineStudio, pstudio, sizeof(engine_studio_api_t));
	pstudio->StudioSetupModel = Hk_StudioSetupModel;

	/* give to client */
	result = cl_funcs.pStudioInterface(version, ppinterface, pstudio);

	/* backup and change client stuff */
	memcpy(&studio, (*ppinterface), sizeof(studio));
	(*ppinterface)->StudioDrawModel = Hk_StudioDrawModel;

	isSoftware = !pstudio->IsHardware();
	if (!isSoftware)
	{
#if defined(_WIN32)
		isOpenGL = wglGetCurrentContext() != NULL;
#else
		isOpenGL = true;
#endif
	}

	return result;
}
