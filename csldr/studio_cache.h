typedef struct
{
	bool mirror_shell;
	bool mirror_model;
	vec3_t origin;
	float fov_override;
} studio_config_t;

typedef struct studio_cache_s
{
	char name[64]; // name of the model file
	struct studio_cache_s *children[4];

	// header fields
	int header_length;

	// configuration
	studio_config_t config;
} studio_cache_t;

extern unsigned int flush_count;

studio_cache_t *GetStudioCache(model_t *model, studiohdr_t *header);
studio_cache_t *EntityStudioCache(cl_entity_t *entity);

void UpdateStudioCaches(void);

void StudioConfigFlush_f(void);
