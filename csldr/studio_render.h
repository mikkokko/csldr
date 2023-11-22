#define MAX_ELIGHTS 3

typedef struct
{
	cl_entity_t *entity;
	studiohdr_t *header;
	model_t *model;
	studio_cache_t *cache;

	mat3x4_t (*bonetransform)[];

	mstudiomodel_t *submodel;
	mem_model_t *mem_submodel;

	float ambientlight;
	float shadelight;
	vec3_t lightcolor;
	vec3_t lightvec;

	int elight_num;
	float elight_pos[MAX_ELIGHTS][4];
	float elight_color[MAX_ELIGHTS][3];

	vec3_t chrome_origin;

	struct studio_shader_s *shader;
} studio_context_t;

typedef struct studio_globals_s
{
	// engine structs set on startup
	cvar_t *r_glowshellfreq;
	cvar_t *gl_fog;
	dlight_t *elights;

	// only for gpu skinning
	GLuint ubo;

	// for cpu chrome, incremented in StudioDrawModel and StudioDrawPlayer hooks
	int drawcount;

	// incremented every frame
	int framecount;

	// GL_FOG_MODE this frame
	GLint fog_mode;
} studio_globals_t;

extern studio_globals_t studio_globals;

void R_StudioInit(void);

void R_StudioNewFrame(void);

void R_StudioInitContext(studio_context_t *ctx, cl_entity_t *entity, model_t *model, studiohdr_t *header);
void R_StudioEntityLight(studio_context_t *ctx);
void R_StudioSetupLighting(studio_context_t *ctx, alight_t *lighting);
void R_StudioSetupRenderer(studio_context_t *ctx);
void R_StudioRestoreRenderer(studio_context_t *ctx);

void R_StudioSetupModel(studio_context_t *ctx, int bodypart_index);
void R_StudioDrawPoints(studio_context_t *ctx);
