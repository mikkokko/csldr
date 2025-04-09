#include "pch.h"

engine_studio_api_t IEngineStudio;
static r_studio_interface_t studio;

#define Z_NEAR 1.0f /* was 4 but that's too far for viewmodels */
#define Z_FAR 4096.0f

bool ShouldMirrorViewmodel(cl_entity_t *vm)
{
	studio_cache_t *cache = EntityStudioCache(vm);
	if (!cache)
		return false;

	return cache->config.mirror_model;
}

static bool PushMirrorViewmodel(cl_entity_t *vm, float *value)
{
	if (!ShouldMirrorViewmodel(vm))
		return false;

	*value = cl_righthand->value;
	cl_righthand->value = (float)!cl_righthand->value;
	return true;
}

static void PopMirrorViewmodel(float value)
{
	cl_righthand->value = value;
}

static void DrawHands(cl_entity_t *weapon, int flags)
{
	bool changed;
	model_t *model;
	cl_entity_t backup;
	static char hands_path[256] = "";
	static bool hands_valid = false;

	if (!*viewmodel_hands->string)
		return;

	/* skip "models/" for comparison */
	changed = strncmp(hands_path + 7, viewmodel_hands->string, sizeof(hands_path) - 7) != 0;
	if (changed)
	{
		/* update path */
		sprintf(hands_path, "models/%s", viewmodel_hands->string);
	}
	else if (!hands_valid)
	{
		/* no change and the last hands were invalid, don't even try */
		return;
	}

	/* mikkotodo: even though Mod_ForName returns null, it doesn't actually
	free the model so we'd need to do that manually... */
	model = IEngineStudio.Mod_ForName(hands_path, false);
	hands_valid = model != NULL;
	if (!hands_valid)
		return;

	/* should probably see what changes and only backup the necessary */
	backup = *weapon;

	/* abuse existing bonemerge functionality */
	weapon->model = model;
	weapon->curstate.movetype = 12; /* MOVETYPE_FOLLOW */

	studio.StudioDrawModel(flags);

	*weapon = backup;
}

static float GetViewmodelFov(void)
{
	studio_cache_t *cache = EntityStudioCache(gEngfuncs.GetViewModel());
	if (!cache)
		return viewmodel_fov->value;

	float scale = (cache->config.fov_override > 0) ? (cache->config.fov_override / 90) : 1.0f;
	return viewmodel_fov->value * scale;
}

static void SetProjectionMatrix(void)
{
	float aspect, fov, f, fn;
	float matrix[4][4];

	if (!canOpenGL)
		return; // won't work

	if (renderActive)
		return; // won't work (renderer does this itself)

	aspect = (float)screenWidth / (float)screenHeight;

	fov = GetViewmodelFov() * fovScale;
	fov = CLAMP(fov, 1, 179);
	fov = 0.75f * tanf(Radians(fov) * 0.5f);

	f = 1.0f / fov;
	fn = (1.0f / (Z_NEAR - Z_FAR));

	memset(matrix, 0, sizeof(matrix));
	matrix[0][0] = f / aspect;
	matrix[1][1] = f;
	matrix[2][2] = (Z_NEAR + Z_FAR) * fn;
	matrix[2][3] = -1;
	matrix[3][2] = 2 * Z_NEAR * Z_FAR * fn;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&matrix[0][0]);
	glMatrixMode(GL_MODELVIEW);
}

static void RestoreProjectionMatrix(void)
{
	if (!canOpenGL)
		return; // won't work

	if (renderActive)
		return; // won't work (renderer does this itself)

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

static int My_StudioDrawModel(int flags)
{
	int result;
	bool mirrored;
	float old_righthand;

	cl_entity_t *entity = IEngineStudio.GetCurrentEntity();

	if (entity != IEngineStudio.GetViewEntity())
		return studio.StudioDrawModel(flags);

	SetProjectionMatrix();

	/* think about inspecting now since we're about to draw the vm */
	InspectThink();

	mirrored = PushMirrorViewmodel(entity, &old_righthand);

	result = studio.StudioDrawModel(flags);

	/* draw hands now that the scene is properly set up */
	DrawHands(entity, flags);

	if (mirrored)
		PopMirrorViewmodel(old_righthand);

	RestoreProjectionMatrix();

	return result;
}

static void Hk_StudioSetupModel(int bodypart, void **ppbodypart, void **ppsubmodel)
{
	cl_entity_t *entity;
	bool viewModel;
	studiohdr_t *studiohdr;
	mstudiobodyparts_t *body;
	int oldbody;
	int current;
	int newbody;
	int teamIndex;

	entity = IEngineStudio.GetCurrentEntity();
	viewModel = (entity == IEngineStudio.GetViewEntity());
	if (!viewModel) /* we don't want to do anything */
	{
		IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);
		return;
	}

	/* check if bodypart is right */
	studiohdr = (studiohdr_t *)entity->model->cache.data;
	if (!studiohdr)
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

	/* bodypart is right one, do arm changing stuff */
	oldbody = entity->curstate.body;

	if (user1 == 4)
		teamIndex = playerInfo[user2].team;
	else
		teamIndex = localTeam;

	newbody = (teamIndex == TEAM_CT) ? 1 : 0;

	current = (entity->curstate.body / body->base) % body->nummodels;
	entity->curstate.body = (entity->curstate.body - (current * body->base) + (newbody * body->base));

	/* set the arm bodygroup */
	IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);

	/* restore state */
	entity->curstate.body = oldbody;
}

int Hk_GetStudioModelInterface(int version,
		r_studio_interface_t **ppinterface,
		engine_studio_api_t *pstudio)
{
	int result;

	/* setup the GL, done here so we can get at pstudio->IsHardware() */
	switch(pstudio->IsHardware())
	{
	case 1:
		canOpenGL = gladLoadGL();
		break;

	case 2:
		canOpenGL = gladLoadGLLoader(D3D_GL_GetProcAddress);
		break;

	default:
		canOpenGL = false;
		break;
	}

	/* let the renderer tap in before us */
	Render_Initialize(pstudio, ppinterface);

	memcpy(&IEngineStudio, pstudio, sizeof(engine_studio_api_t));
	pstudio->StudioSetupModel = Hk_StudioSetupModel;

	/* give to client */
	result = cl_funcs.pStudioInterface(version, ppinterface, pstudio);

	/* backup and change client stuff */
	memcpy(&studio, (*ppinterface), sizeof(studio));
	(*ppinterface)->StudioDrawModel = My_StudioDrawModel;

	return result;
}
