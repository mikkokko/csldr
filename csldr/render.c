#include "pch.h"

/* in export.c */
EXPORT int HUD_AddEntity(int type, cl_entity_t *ent, const char *modelname);
EXPORT void HUD_DrawNormalTriangles(void);
EXPORT void HUD_DrawTransparentTriangles(void);

/* must match with the definition in renderer's loader.cpp */
typedef struct
{
	int(*AddEntity)(int, cl_entity_t *, const char *);
	void(*DrawNormalTriangles)(void);
	void(*DrawTransparentTriangles)(void);
} clientInterface_t;

/* must match with the definition in renderer's loader.cpp */
typedef struct
{
	void (*ModifyEngfuncs)(cl_enginefunc_t *);
	void (*Initialize)(engine_studio_api_t *, r_studio_interface_t **);
	int (*BeginFrame)(void);
	void (*RenderScene)(const renderParams_t *);
	int (*AddEntity)(int, cl_entity_t *);
	addEntityCallback_t(*GetAddEntityCallback)(addEntityCallback_t);
	void (*PreDrawHud)(void);
	void (*PostDrawHud)(int, int);
} renderInterface_t;

static clientInterface_t s_clientInterface =
{
	HUD_AddEntity,
	HUD_DrawNormalTriangles,
	HUD_DrawTransparentTriangles
};

static void *s_renderHandle;
static renderInterface_t s_renderInterface;

typedef int (*loaderConnect_t)(clientInterface_t *, renderInterface_t *, int, int);

void Render_Load(void)
{
	char path[512];
	size_t length = Plat_CurrentModuleName(path, sizeof(path));
	if (!length)
	{
		/* too bad */
		return;
	}

	/* tacky, but it'll work... */
	char *start = strrchr(path, '/');
	if (!start)
	{
		start = strrchr(path, '\\');
		if (!start)
		{
			/* too bad */
			return;
		}
	}

	if (strncmp(start + 1, "client", 6))
	{
		/* too bad */
		return;
	}

	memcpy(start + 1, "render", 6);

	s_renderHandle = Plat_Dlopen(path);
	if (!s_renderHandle)
	{
		/* too bad */
		return;
	}

	loaderConnect_t connect = (loaderConnect_t)Plat_Dlsym(s_renderHandle, "LoaderConnect");
	if (!connect)
	{
		/* too bad */
		Plat_Dlclose(s_renderHandle);
		s_renderHandle = NULL;
		return;
	}

	if (!connect(&s_clientInterface , &s_renderInterface, sizeof(renderInterface_t), sizeof(renderParams_t)))
	{
		/* too bad */
		Plat_Dlclose(s_renderHandle);
		s_renderHandle = NULL;
		return;
	}

	/* looks like we're good... */
}

void Render_Unload(void)
{
	if (s_renderHandle)
	{
		Plat_Dlclose(s_renderHandle);
		s_renderHandle = NULL;
	}
}

void Render_ModifyEngfuncs(cl_enginefunc_t *engfuncs)
{
	if (s_renderInterface.ModifyEngfuncs)
	{
		s_renderInterface.ModifyEngfuncs(engfuncs);
	}
}

void Render_Initialize(engine_studio_api_t *studio, r_studio_interface_t **pinterface)
{
	if (s_renderInterface.Initialize)
	{
		s_renderInterface.Initialize(studio, pinterface);
	}
}

int Render_BeginFrame(void)
{
	if (s_renderInterface.BeginFrame)
	{
		return s_renderInterface.BeginFrame();
	}

	return 0;
}

void Render_RenderScene(const renderParams_t *params)
{
	if (s_renderInterface.RenderScene)
	{
		s_renderInterface.RenderScene(params);
	}
}

int Render_AddEntity(int type, cl_entity_t *entity)
{
	if (s_renderInterface.AddEntity)
	{
		return s_renderInterface.AddEntity(type, entity);
	}

	return 0;
}

addEntityCallback_t Render_GetAddEntityCallback(addEntityCallback_t original)
{
	if (s_renderInterface.GetAddEntityCallback)
	{
		return s_renderInterface.GetAddEntityCallback(original);
	}

	return original;
}

void Render_PreDrawHud(void)
{
	if (s_renderInterface.PreDrawHud)
	{
		s_renderInterface.PreDrawHud();
	}
}

void Render_PostDrawHud(int screenWidth, int screenHeight)
{
	if (s_renderInterface.PostDrawHud)
	{
		s_renderInterface.PostDrawHud(screenWidth, screenHeight);
	}
}
