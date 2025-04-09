/* if this doesn't match Render::Params, enjoy the segfaults */
typedef struct
{
	vec3_t origin;
	vec3_t angles;
	vec3_t crosshairAngle;
	float fov;
	float aspectRatio;
	movevars_t *movevars;
} renderParams_t;

typedef int(*addEntityCallback_t)(cl_entity_t *);

void Render_Load(void);
void Render_Unload(void);

void Render_ModifyEngfuncs(cl_enginefunc_t *engfuncs);
void Render_Initialize(engine_studio_api_t *studio, r_studio_interface_t **pinterface);
int Render_BeginFrame(void);
void Render_RenderScene(const renderParams_t *params);
int Render_AddEntity(int type, cl_entity_t *entity);
addEntityCallback_t Render_GetAddEntityCallback(addEntityCallback_t original);
void Render_PreDrawHud(void);
void Render_PostDrawHud(int screenWidth, int screenHeight);
