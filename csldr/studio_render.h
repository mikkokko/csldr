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

	struct studio_shader_s *shader;
} studio_context_t;

// incremented in StudioDrawModel and StudioDrawPlayer hooks
extern int studio_drawcount;

void R_StudioInit(void);

void R_StudioNewFrame(void);

void R_StudioInitContext(studio_context_t *ctx, cl_entity_t *entity, model_t *model, studiohdr_t *header);
void R_StudioEntityLight(studio_context_t *ctx);
void R_StudioSetupLighting(studio_context_t *ctx, alight_t *lighting);
void R_StudioSetupRenderer(studio_context_t *ctx);
void R_StudioRestoreRenderer(studio_context_t *ctx);

void R_StudioSetupModel(studio_context_t *ctx, int bodypart_index);
void R_StudioDrawPoints(studio_context_t *ctx);
