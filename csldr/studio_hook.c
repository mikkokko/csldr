#include "pch.h"

// 3*4*128 for bone matrices and then some
#define REQUIRED_UNIFORM_COMPONENTS 1600

bool studio_fastpath;
bool studio_uboable;

static int s_max_vertex_uniform_components;

static studio_context_t context;

static cvar_t *studio_fastpath_toggle;
static cvar_t *studio_fastpath_min_polys;

static studiohdr_t *s_header;
static model_t *s_model;

static bool skip_fastpath;

#define FASTPATH_ENABLED (studio_fastpath && studio_fastpath_toggle->value)

static void Hk_StudioSetHeader(void *header)
{
	IEngineStudio.StudioSetHeader(header);

	if (FASTPATH_ENABLED)
	{
		s_header = (studiohdr_t *)header;
	}
}

static void Hk_SetRenderModel(model_t *model)
{
	IEngineStudio.SetRenderModel(model);

	if (FASTPATH_ENABLED)
	{
		s_model = model;
	}
}

static model_t *last_model;

static void *Hk_Mod_Extradata(model_t *model)
{
	last_model = model;
	return IEngineStudio.Mod_Extradata(model);
}

static model_t *DeduceModel(studiohdr_t *header)
{
	assert(header);

	if (last_model->cache.data == header)
		return last_model;

	Plat_Error("Could not find model for header %s\n", header->name);
	return NULL; // never reached
}

static void Hk_StudioEntityLight(alight_t *lighting)
{
	if (FASTPATH_ENABLED)
	{
		// done in Hk_StudioSetupLighting
	}
	else
	{
		IEngineStudio.StudioEntityLight(lighting);
	}
}

static void Hk_StudioSetupLighting(alight_t *lighting)
{
	if (FASTPATH_ENABLED)
	{
		if (!s_model || (s_model->cache.data && (s_model->cache.data != s_header)))
		{
			// this happens with player weapon models... Mod_Extradata is called right
			// before this so we should be able to get the model pointer from there
			s_model = DeduceModel(s_header);
		}

		assert(s_model);
		assert(s_header);

		studio_cache_t *cache = GetStudioCache(s_model, s_header);
		skip_fastpath = (!cache->needs_renderer) && (cache->max_drawn_polys < studio_fastpath_min_polys->value);
		if (skip_fastpath)
		{
			IEngineStudio.StudioSetupLighting(lighting);
			return;
		}

		R_StudioInitContext(&context, IEngineStudio.GetCurrentEntity(), s_model, s_header, cache);

		// moved here because of skip_fastpath
		R_StudioEntityLight(&context);

		R_StudioSetupLighting(&context, lighting);
	}
	else
	{
		IEngineStudio.StudioSetupLighting(lighting);
	}
}

void Hk_StudioSetupModel(int bodypart, void **ppbodypart, void **ppsubmodel)
{
	IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);

	if (FASTPATH_ENABLED && !skip_fastpath)
	{
		R_StudioSetupModel(&context, bodypart);
	}
}

static void Hk_StudioDrawPoints(void)
{
	if (FASTPATH_ENABLED && !skip_fastpath)
	{
		R_StudioDrawPoints(&context);
	}
	else
	{
		IEngineStudio.StudioDrawPoints();
	}
}

static void Hk_StudioDrawHulls(void)
{
	if (FASTPATH_ENABLED && !skip_fastpath)
	{
		// not implemented for fast path
	}
	else
	{
		IEngineStudio.StudioDrawHulls();
	}
}

static void Hk_StudioDrawAbsBBox(void)
{
	if (FASTPATH_ENABLED && !skip_fastpath)
	{
		// not implemented for fast path
	}
	else
	{
		IEngineStudio.StudioDrawAbsBBox();
	}
}

static void Hk_StudioDrawBones(void)
{
	if (FASTPATH_ENABLED && !skip_fastpath)
	{
		// not implemented for fast path
	}
	else
	{
		IEngineStudio.StudioDrawBones();
	}
}

static void Hk_SetupRenderer(int rendermode)
{
	IEngineStudio.SetupRenderer(rendermode);

	if (FASTPATH_ENABLED && !skip_fastpath)
	{
		R_StudioSetupRenderer(&context);
	}
}

static void Hk_RestoreRenderer(void)
{
	if (FASTPATH_ENABLED && !skip_fastpath)
	{
		R_StudioRestoreRenderer(&context);
	}

	IEngineStudio.RestoreRenderer();
}

static void StudioInfo_f(void)
{
	gEngfuncs.Con_Printf("OpenGL version: %s\n", glGetString(GL_VERSION));
	gEngfuncs.Con_Printf("GL_ARB_uniform_buffer_object: %s\n", GLAD_GL_ARB_uniform_buffer_object ? "available" : "not available");
	gEngfuncs.Con_Printf("GL_MAX_VERTEX_UNIFORM_COMPONENTS: %d\n", s_max_vertex_uniform_components);
	gEngfuncs.Con_Printf("Fast path: %s\n", studio_fastpath ? "available" : "not available");

	int num_models, max_models;
	StudioCacheStats(&num_models, &max_models);
	gEngfuncs.Con_Printf("Models cached: %d / %d\n", num_models, max_models);

	size_t used, size;
	Mem_GetInfo(&used, &size);
	double mb_used = used * (1.0 / 1024.0 / 1024.0);
	double mb_size = size * (1.0 / 1024.0 / 1024.0);
	gEngfuncs.Con_Printf("Memory usage: %.3f MB / %.3f MB\n", mb_used, mb_size);
}

void HookEngineStudio(engine_studio_api_t *studio)
{
	switch(studio->IsHardware())
	{
	default:
		canOpenGL = false;
		break;

	case 1:
		canOpenGL = gladLoadGL();
		break;

	case 2:
		// special d3d path
		canOpenGL = gladLoadGLLoader(D3D_GL_GetProcAddress);
		break;
	}

	if (!canOpenGL)
		return; // won't work

	// see if we can do gpu skinning without ubos
	glad_glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &s_max_vertex_uniform_components);

	// see if we can do fast path
	if (GLAD_GL_VERSION_2_1)
	{
		if (GLVersion.major >= 3 && GLVersion.minor >= 2 && GLAD_GL_ARB_uniform_buffer_object)
		{
			// opengl 3.2 supports compatibility profiles so we should
			// be able to use layout(std140) in our otherwise glsl 1.20 shader
			// see R_StudioSelectShader
			studio_fastpath = true;
			studio_uboable = true;
		}
		else if (s_max_vertex_uniform_components >= REQUIRED_UNIFORM_COMPONENTS)
		{
			studio_fastpath = true;
			studio_uboable = false;
		}
		else if (GLAD_GL_ARB_uniform_buffer_object)
		{
			// probably won't work but ok (see R_StudioSelectShader)
			studio_fastpath = true;
			studio_uboable = true;
		}
	}

	gEngfuncs.pfnAddCommand("studio_info", StudioInfo_f);

	// mikkotodo fix this properly
	studio->StudioSetupModel = My_StudioSetupModel; // reroute for bodygroup changing

	if (studio_fastpath)
	{
		studio->StudioSetHeader = Hk_StudioSetHeader;
		studio->Mod_Extradata = Hk_Mod_Extradata;
		studio->SetRenderModel = Hk_SetRenderModel;
		studio->StudioEntityLight = Hk_StudioEntityLight;
		studio->StudioSetupLighting = Hk_StudioSetupLighting;
		studio->StudioDrawPoints = Hk_StudioDrawPoints;
		studio->StudioDrawHulls = Hk_StudioDrawHulls;
		studio->StudioDrawAbsBBox = Hk_StudioDrawAbsBBox;
		studio->StudioDrawBones = Hk_StudioDrawBones;
		studio->SetupRenderer = Hk_SetupRenderer;
		studio->RestoreRenderer = Hk_RestoreRenderer;

		R_StudioInit();

		studio_fastpath_toggle = gEngfuncs.pfnRegisterVariable("studio_fastpath", "0", FCVAR_ARCHIVE);
		studio_fastpath_min_polys = gEngfuncs.pfnRegisterVariable("studio_fastpath_min_polys", "100", FCVAR_ARCHIVE);

		gEngfuncs.pfnAddCommand("studio_config_flush", StudioConfigFlush_f);
	}
}
