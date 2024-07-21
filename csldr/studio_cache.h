typedef struct
{
	unsigned ofs_indices;
	unsigned num_indices;
} mem_mesh_t;

typedef struct mem_model_s
{
	mem_mesh_t *meshes;
} mem_model_t;

typedef struct
{
	mem_model_t *models;
} mem_bodypart_t;

// we could pack the components to halve the size of vertices, right? WRONG!!!! that completely tanks peformance on old ATI APUs
typedef struct
{
	float pos[3];
	float norm[3];
	float texcoord[2];
	float bones[2];
} studio_vert_t;

typedef struct
{
	char name[64];
	GLuint diffuse;
	
	// renderer structures
	int num_elements;
	unsigned *counts;
	void **offsets;
} mem_texture_t;

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
	bool needs_renderer;
	studio_config_t config;

	// renderer specfic stuff
	int max_drawn_polys;
	int num_gpubones;
	byte map_gpubones[128];

	mem_texture_t *textures;
	int numtextures;

	mem_bodypart_t *bodyparts;

	GLuint studio_vbo;
	GLuint studio_ebo;
} studio_cache_t;

extern unsigned int flush_count;

studio_cache_t *GetStudioCache(model_t *model, studiohdr_t *header);
studio_cache_t *EntityStudioCache(cl_entity_t *entity);

void UpdateStudioCaches(void);

void StudioCacheStats(int *count, int *max);
void StudioConfigFlush_f(void);

studiohdr_t *R_LoadTextures(model_t *model, studiohdr_t *header);
