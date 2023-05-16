typedef struct
{
	unsigned ofs_indices;
	unsigned num_indices;

	// only for cpu skinning
	unsigned ofs_verts;
	unsigned num_verts;
} mem_mesh_t;

typedef struct mem_model_s
{
	mem_mesh_t *meshes;

	// only for cpu skinning
	unsigned ofs_verts;
	unsigned num_verts;
} mem_model_t;

typedef struct
{
	mem_model_t *models;
} mem_bodypart_t;

typedef struct
{
	float pos[3];
	float norm[3];
	float texcoord[2];
} studio_cpu_vert_t;

typedef struct
{
	float pos[3];
	float norm[3];
	float texcoord[2];
	float bones[2];
} studio_gpu_vert_t;

// only for cpu skinning
typedef struct
{
	byte bones[2];
} studio_vertbone_t;

typedef struct
{
	char name[64];
	GLuint diffuse;
} mem_texture_t;

typedef struct studio_cache_s
{
	uint32 hash;

	// name of the configuration file
	char config_path[64];

	// renderer specfic stuff
	mem_texture_t *textures;
	int numtextures;

	mem_bodypart_t *bodyparts;

	GLuint studio_vbo;
	GLuint studio_ebo;

	// only for cpu skinning
	studio_cpu_vert_t *verts;
	studio_vertbone_t *vertbones;
} studio_cache_t;

studio_cache_t *GetStudioCache(model_t *model, studiohdr_t *header);

// mikkotodo might be necessary again if we need extremely expensive tangent calc
void UpdateStudioCaches(void);

void StudioCacheStats(int *count, int *max);
void StudioConfigFlush_f(void);

studiohdr_t *R_LoadTextures(model_t *model, studiohdr_t *header);
