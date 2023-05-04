#include "pch.h"

#define CACHE_ID (('c' << 0) + ('s' << 8) + ('l' << 16) + ('d' << 24))
#define CACHE_VERSION (('r' << 0) + ('m' << 8) + ('d' << 16) + ('l' << 24))

#define MAX_CACHED 4096

static studio_cache_t studio_cache[MAX_CACHED];
static int num_cache;

// mikkotodo make somewhat dynamic?
// mikkotodo make sure that these won't overflow
#define BUILD_NUM_VERTICES (102400)
#define BUILD_NUM_INDICES (BUILD_NUM_VERTICES * 3)

typedef struct
{
	int num_verts;
	int num_indices;
	GLuint indices[BUILD_NUM_INDICES];
} base_build_buffer_t;

typedef struct
{
	base_build_buffer_t base;
	studio_cpu_vert_t verts[BUILD_NUM_VERTICES];
	studio_vertbone_t vertbones[BUILD_NUM_VERTICES];
} cpu_build_buffer_t;

typedef struct
{
	base_build_buffer_t base;
	studio_gpu_vert_t verts[BUILD_NUM_VERTICES];
} gpu_build_buffer_t;

// FNV-1a hash
#define FNV_OFFSET_BASIS32 0x811c9dc5
#define FNV_PRIME32 0x01000193

static uint32 HashData(const void *data, int size)
{
	uint32 hash = FNV_OFFSET_BASIS32;

	for (int i = 0; i < size; i++)
	{
		hash ^= ((const unsigned char *)data)[i];
		hash *= FNV_PRIME32;
	}

	return hash;
}

static void ParseTricmds(base_build_buffer_t *build, short *tricmds, vec3_t *vertices, vec3_t *normals, byte *vertinfo, byte *norminfo, float s, float t)
{
	while (1)
	{
		int count = *tricmds++;
		if (!count)
			break;

		bool trifan = false;

		if (count < 0)
		{
			trifan = true;
			count = -count;
		}

		int offset = build->num_verts;
		build->num_verts += count;

		if (studio_gpuskin)
		{
			gpu_build_buffer_t *gpu_build = (gpu_build_buffer_t *)build;
			studio_gpu_vert_t *vert = &gpu_build->verts[offset];

			for (int l = 0; l < count; l++)
			{
				for (int m = 0; m < 3; m++)
				{
					vert->pos[m] = vertices[tricmds[0]][m];
					vert->norm[m] = normals[tricmds[1]][m];
				}

				vert->texcoord[0] = s * tricmds[2];
				vert->texcoord[1] = t * tricmds[3];

				vert->bones[0] = vertinfo[tricmds[0]];
				vert->bones[1] = norminfo[tricmds[1]];

				tricmds += 4;
				vert++;
			}
		}
		else
		{
			cpu_build_buffer_t *cpu_build = (cpu_build_buffer_t *)build;
			studio_cpu_vert_t *vert = &cpu_build->verts[offset];
			studio_vertbone_t *vertbone = &cpu_build->vertbones[offset];

			for (int l = 0; l < count; l++)
			{
				for (int m = 0; m < 3; m++)
				{
					vert->pos[m] = vertices[tricmds[0]][m];
					vert->norm[m] = normals[tricmds[1]][m];
				}

				vert->texcoord[0] = s * tricmds[2];
				vert->texcoord[1] = t * tricmds[3];

				vertbone->bones[0] = vertinfo[tricmds[0]];
				vertbone->bones[1] = norminfo[tricmds[1]];

				tricmds += 4;
				vert++;
				vertbone++;
			}
		}

		if (trifan)
		{
			for (int i = 2; i < count; i++)
			{
				build->indices[build->num_indices++] = offset;
				build->indices[build->num_indices++] = offset + i - 1;
				build->indices[build->num_indices++] = offset + i;
			}
		}
		else
		{
			for (int i = 2; i < count; i++)
			{
				if (!(i % 2))
				{
					build->indices[build->num_indices++] = offset + i - 2;
					build->indices[build->num_indices++] = offset + i - 1;
					build->indices[build->num_indices++] = offset + i;
				}
				else
				{
					build->indices[build->num_indices++] = offset + i - 1;
					build->indices[build->num_indices++] = offset + i - 2;
					build->indices[build->num_indices++] = offset + i;
				}
			}
		}
	}
}

static void BuildStudioVBO(studio_cache_t *cache, model_t *model, studiohdr_t *header)
{
	static base_build_buffer_t *build = NULL;

	if (!build)
	{
		if (studio_gpuskin)
			build = (base_build_buffer_t *)Mem_Alloc(sizeof(gpu_build_buffer_t));
		else
			build = (base_build_buffer_t *)Mem_Alloc(sizeof(cpu_build_buffer_t));
	}

	build->num_verts = 0;
	build->num_indices = 0;

	mstudiobodyparts_t *bodyparts = (mstudiobodyparts_t *)((byte *)header + header->bodypartindex);

	studiohdr_t *textureheader = R_LoadTextures(model, header);
	short *skins = (short *)((byte *)textureheader + textureheader->skinindex);
	mstudiotexture_t *textures = (mstudiotexture_t *)((byte *)textureheader + textureheader->textureindex);

	cache->bodyparts = (mem_bodypart_t *)Mem_Alloc(sizeof(*cache->bodyparts) * header->numbodyparts);

	for (int i = 0; i < header->numbodyparts; i++)
	{
		mstudiobodyparts_t *bodypart = &bodyparts[i];
		mstudiomodel_t *models = (mstudiomodel_t *)((byte *)header + bodypart->modelindex);

		mem_bodypart_t *mem_bodypart = &cache->bodyparts[i];
		mem_bodypart->models = (mem_model_t *)Mem_Alloc(sizeof(*mem_bodypart->models) * bodypart->nummodels);

		for (int j = 0; j < bodypart->nummodels; j++)
		{
			mstudiomodel_t *submodel = &models[j];
			mstudiomesh_t *meshes = (mstudiomesh_t *)((byte *)header + submodel->meshindex);

			vec3_t *vertices = (vec3_t *)((byte *)header + submodel->vertindex);
			vec3_t *normals = (vec3_t *)((byte *)header + submodel->normindex);

			byte *vertinfo = (byte *)((byte *)header + submodel->vertinfoindex);
			byte *norminfo = (byte *)((byte *)header + submodel->norminfoindex);

			mem_model_t *mem_model = &mem_bodypart->models[j];
			mem_model->meshes = (mem_mesh_t *)Mem_Alloc(sizeof(*mem_model->meshes) * submodel->nummesh);

			// only for cpu skinning
			int vert_offset = build->num_verts;

			for (int k = 0; k < submodel->nummesh; k++)
			{
				mstudiomesh_t *mesh = &meshes[k];
				short *tricmds = (short *)((byte *)header + mesh->triindex);

				float s = 1.0f / (float)textures[skins[mesh->skinref]].width;
				float t = 1.0f / (float)textures[skins[mesh->skinref]].height;

				int index_offset = build->num_indices;
				
				// only for cpu skinning
				int vert_offset2 = build->num_verts;

				ParseTricmds(build, tricmds, vertices, normals, vertinfo, norminfo, s, t);

				mem_mesh_t *mem_mesh = &mem_model->meshes[k];
				mem_mesh->ofs_indices = index_offset * sizeof(*build->indices);
				mem_mesh->num_indices = build->num_indices - index_offset;

				// only for cpu skinning
				mem_mesh->ofs_verts = vert_offset2;
				mem_mesh->num_verts = build->num_verts - vert_offset2;
			}

			// only for cpu skinning
			mem_model->ofs_verts = vert_offset;
			mem_model->num_verts = build->num_verts - vert_offset;
		}
	}

	glGenBuffers(1, &cache->studio_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cache->studio_vbo);

	if (studio_gpuskin)
	{
		gpu_build_buffer_t *cpu_build = (gpu_build_buffer_t *)build;
		glBufferData(GL_ARRAY_BUFFER, build->num_verts * sizeof(studio_gpu_vert_t), cpu_build->verts, GL_STATIC_DRAW);
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, build->num_verts * sizeof(studio_cpu_vert_t), NULL, GL_DYNAMIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &cache->studio_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cache->studio_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*build->indices) * build->num_indices, build->indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (!studio_gpuskin)
	{
		cpu_build_buffer_t *cpu_build = (cpu_build_buffer_t *)build;

		cache->verts = (studio_cpu_vert_t *)Mem_Alloc(sizeof(studio_cpu_vert_t) * build->num_verts);
		memcpy(cache->verts, cpu_build->verts, sizeof(studio_cpu_vert_t) * build->num_verts);

		cache->vertbones = (studio_vertbone_t *)Mem_Alloc(sizeof(studio_vertbone_t) * build->num_verts);
		memcpy(cache->vertbones, cpu_build->vertbones, sizeof(studio_vertbone_t) * build->num_verts);
	}
}

#if 0 // disable for now

/* this is ok for us because we know the buffer sizes */
static void CopyWithoutExtension(char *dst, const char *src)
{
	while (*src && *src != '.')
		*dst++ = *src++;

	*dst = '\0';
}

static GLuint LoadTexture(const char *path)
{
	// usehunk 2 = Hunk_TempAlloc
	int size;
	byte *filedata = gEngfuncs.COM_LoadFile(path, 2, &size);
	if (!filedata)
		return 0;

	int w, h, comp;
	uint8 *data = Image_LoadTGA(filedata, size, &w, &h, &comp, 4);
	if (!data)
		return 0;

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // mikkotodo might break stuff!!!

	// mikkotodo mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	free(data);

	return texture;
}

#endif

static void LoadTextures(studio_cache_t *cache, model_t *model, studiohdr_t *header)
{
#if 0 // disable for now
	char modelname[64];
	CopyWithoutExtension(modelname, model->name); /* will fit */
#endif

	studiohdr_t *textureheader = R_LoadTextures(model, header);
	mstudiotexture_t *textures = (mstudiotexture_t *)((byte *)textureheader + textureheader->textureindex);

	cache->textures = (mem_texture_t *)Mem_Alloc(sizeof(mem_texture_t) * textureheader->numtextures);

	for (int i = 0; i < textureheader->numtextures; i++)
	{
#if 0 // disable for now
		char texname[64];
		char texpath[256];

		mstudiotexture_t *texture = &textures[i];
		CopyWithoutExtension(texname, texture->name);

		sprintf(texpath, "textures/%s/%s.tga", modelname, texname);
		cache->textures[i].diffuse = LoadTexture(texpath);
#else
		mstudiotexture_t *texture = &textures[i];

		cache->textures[i].diffuse = 0;
#endif
	}
}

static void BuildStudioCache(studio_cache_t *cache, model_t *model, studiohdr_t *header)
{
	cache->hash = HashData(header, sizeof(*header));
	LoadTextures(cache, model, header);
	BuildStudioVBO(cache, model, header);
}

studio_cache_t *GetStudioCache(model_t *model, studiohdr_t *header)
{
	// see if the cache pointer is in the header
	if (header->id == CACHE_ID && header->version == CACHE_VERSION)
		return &studio_cache[header->length];

	// see if this model is cached even
	uint32 hash = HashData(header, sizeof(*header));

	// mikkotodo revisit? slow but probably doesn't matter
	for (int i = 0; i < num_cache; i++)
	{
		if (studio_cache[i].hash == hash)
		{
			// ok, update header
			header->id = CACHE_ID;
			header->version = CACHE_VERSION;
			header->length = i;
			return &studio_cache[i];
		}
	}

	if (num_cache >= MAX_CACHED)
	{
		Plat_Error("Studio model cache full");
		return NULL; // not reached
	}

	// cache the model
	int index = num_cache++;
	studio_cache_t *cache = &studio_cache[index];

	BuildStudioCache(cache, model, header);

	// update header
	header->id = CACHE_ID;
	header->version = CACHE_VERSION;
	header->length = index;

	return cache;
}

// mikkotodo might be necessary again if we need extremely expensive tangent calc
void UpdateStudioCaches(void)
{
	static model_t *last_world = NULL;

	model_t *world = gEngfuncs.hudGetModelByIndex(1);
	if (world == last_world)
		return;

	last_world = world;
	if (!world)
		return;

	// some crackhead engines might have a greater model limit
	// so only break when GetModelByIndex returns null
	for (int i = 1; ; i++)
	{
		model_t *model = IEngineStudio.GetModelByIndex(i);
		if (!model)
			break;

		if (model->type != mod_studio)
			continue;

		studiohdr_t *header = (studiohdr_t *)IEngineStudio.Mod_Extradata(model);
		(void)GetStudioCache(model, header);
	}
}

void StudioCacheStats(int *count, int *max)
{
	*count = num_cache;
	*max = MAX_CACHED;
}
