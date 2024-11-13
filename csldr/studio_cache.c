#include "pch.h"

#define CACHE_ID (('c' << 0) + ('s' << 8) + ('l' << 16) + ('d' << 24))
#define CACHE_VERSION (('r' << 0) + ('m' << 8) + ('d' << 16) + ('l' << 24))

#define MAX_CACHED 4096

static int cache_count;
static studio_cache_t cache_array[MAX_CACHED];
static studio_cache_t *cache_head;

unsigned int flush_count;

typedef struct
{
	unsigned num_verts;
	studio_vert_t *verts;
	unsigned num_indices;
	GLuint *indices;
} build_buffer_t;

static void ParseTricmds(build_buffer_t *build,
	short *tricmds,
	vec3_t *vertices,
	vec3_t *normals,
	byte *vertinfo,
	byte *norminfo,
	float s,
	float t,
	byte *is_rigged_bone)
{
	while (1)
	{
		int value = *tricmds++;
		if (!value)
			break;

		bool trifan = false;

		if (value < 0)
		{
			trifan = true;
			value = -value;
		}

		unsigned count = (unsigned)value;

		unsigned offset = build->num_verts;
		build->num_verts += count;

		studio_vert_t *vert = &build->verts[offset];

		for (unsigned l = 0; l < count; l++)
		{
			for (unsigned m = 0; m < 3; m++)
			{
				vert->pos[m] = vertices[tricmds[0]][m];
				vert->norm[m] = normals[tricmds[1]][m];
			}

			vert->texcoord[0] = s * tricmds[2];
			vert->texcoord[1] = t * tricmds[3];

			vert->bones[0] = vertinfo[tricmds[0]];
			vert->bones[1] = norminfo[tricmds[1]];

			is_rigged_bone[vertinfo[tricmds[0]]] = true;
			is_rigged_bone[norminfo[tricmds[1]]] = true;

			tricmds += 4;
			vert++;
		}

		if (trifan)
		{
			for (unsigned i = 2; i < count; i++)
			{
				build->indices[build->num_indices++] = offset;
				build->indices[build->num_indices++] = offset + i - 1;
				build->indices[build->num_indices++] = offset + i;
			}
		}
		else
		{
			for (unsigned i = 2; i < count; i++)
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

static int CountVertsTricmds(short *tricmds)
{
	int result = 0;

	while (1)
	{
		int value = *tricmds++;
		if (!value)
			break;

		if (value < 0)
			value = -value;

		result += value;

		tricmds += (4 * value);
	}

	return result;
}

static int CountVerts(studiohdr_t *header, int *pmax_drawn_polys)
{
	int total_verts = 0;
	int max_drawn_polys = 0;

	mstudiobodyparts_t *bodyparts = (mstudiobodyparts_t *)((byte *)header + header->bodypartindex);

	for (int i = 0; i < header->numbodyparts; i++)
	{
		mstudiobodyparts_t *bodypart = &bodyparts[i];
		mstudiomodel_t *models = (mstudiomodel_t *)((byte *)header + bodypart->modelindex);

		int max_bodypart_polys = 0;

		for (int j = 0; j < bodypart->nummodels; j++)
		{
			mstudiomodel_t *submodel = &models[j];
			mstudiomesh_t *meshes = (mstudiomesh_t *)((byte *)header + submodel->meshindex);

			int submodel_polys = 0;

			for (int k = 0; k < submodel->nummesh; k++)
			{
				mstudiomesh_t *mesh = &meshes[k];
				short *tricmds = (short *)((byte *)header + mesh->triindex);
				total_verts += CountVertsTricmds(tricmds);
				submodel_polys += mesh->numtris;
			}

			max_bodypart_polys = MAX(max_bodypart_polys, submodel_polys);
		}

		max_drawn_polys += max_bodypart_polys;
	}

	*pmax_drawn_polys = max_drawn_polys;
	return total_verts;
}

static void BuildStudioVBO(studio_cache_t *cache, model_t *model, studiohdr_t *header)
{
	// mikkotodo revisit, for some reason meshes won't draw when the index offset is 0???
	const int index_reserve = 1;

	int total_verts = CountVerts(header, &cache->max_drawn_polys);

	build_buffer_t build;
	build.num_verts = 0;
	build.verts = (studio_vert_t *)Mem_TempAlloc(sizeof(*build.verts) * total_verts);
	build.num_indices = index_reserve;
	build.indices = (GLuint *)Mem_TempAlloc(index_reserve + sizeof(*build.indices) * total_verts * 3);

	mstudiobodyparts_t *bodyparts = (mstudiobodyparts_t *)((byte *)header + header->bodypartindex);

	studiohdr_t *textureheader = R_LoadTextures(model, header);
	short *skins = (short *)((byte *)textureheader + textureheader->skinindex);
	mstudiotexture_t *textures = (mstudiotexture_t *)((byte *)textureheader + textureheader->textureindex);

	cache->bodyparts = (mem_bodypart_t *)Mem_Alloc(sizeof(*cache->bodyparts) * header->numbodyparts);

	byte is_rigged_bone[128]; // keep track of bones that are used for skinning
	memset(is_rigged_bone, 0, sizeof(is_rigged_bone));

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

			for (int k = 0; k < submodel->nummesh; k++)
			{
				mstudiomesh_t *mesh = &meshes[k];
				mstudiotexture_t *texture = &textures[skins[mesh->skinref]];
				short *tricmds = (short *)((byte *)header + mesh->triindex);

				float s = 1.0f / (float)texture->width;
				float t = 1.0f / (float)texture->height;

				unsigned index_offset = build.num_indices;

				// save texture flags for shader selection
				cache->texflags |= texture->flags;

				ParseTricmds(&build, tricmds, vertices, normals, vertinfo, norminfo, s, t, is_rigged_bone);

				mem_mesh_t *mem_mesh = &mem_model->meshes[k];
				mem_mesh->ofs_indices = index_offset * sizeof(*build.indices);
				mem_mesh->num_indices = build.num_indices - index_offset;
			}
		}
	}

	// build bone remap arrays
	byte bone_remap[128];
	for (int i = 0; i < header->numbones; i++)
	{
		if (is_rigged_bone[i])
		{
			bone_remap[i] = cache->num_gpubones;
			cache->map_gpubones[cache->num_gpubones++] = i;
		}
	}

	// remap bone indices to the correct ones
	for (unsigned i = 0; i < build.num_verts; i++)
	{
		studio_vert_t *vert = &build.verts[i];
		vert->bones[0] = bone_remap[(int)vert->bones[0]];
		vert->bones[1] = bone_remap[(int)vert->bones[1]];
	}

	glGenBuffers(1, &cache->studio_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cache->studio_vbo);
	glBufferData(GL_ARRAY_BUFFER, build.num_verts * sizeof(studio_vert_t), build.verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &cache->studio_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cache->studio_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*build.indices) * build.num_indices, build.indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Mem_TempFree(build.verts);
	Mem_TempFree(build.indices);
}

static void ParseTextures(studio_cache_t *cache, keyValue_t *key, bool flush, const char *config_path)
{
	if (!studio_fastpath)
		return;

	for (int i = 0; i < key->numSubkeys; i++)
	{
		bool found = false;
		keyValue_t *subkey = &key->subkeys[i];

		for (int j = 0; j < cache->numtextures; j++)
		{
			mem_texture_t *texture = &cache->textures[j];

			if (!strcmp(subkey->name, texture->name))
			{
				StudioLoadTexture(subkey, texture, flush);
				found = true;
				break;
			}
		}

		if (!found)
		{
			gEngfuncs.Con_Printf("Texture %s not overriding an existing one in %s, ignoring\n",
				subkey->name, config_path);
		}
		else
		{
			// using custom textures, need to use the custom renderer
			cache->needs_renderer = true;
		}
	}
}

static bool ParseBoolean(keyValue_t *key, const char *path)
{
	if (key->type != keyValueString)
	{
		gEngfuncs.Con_Printf("Expected boolean value for option %s in %s\n", key->name, path);
		return false;
	}

	// dumb as fuck
	return atoi(key->string) ? true : false;
}

static float ParseFloat(keyValue_t *key, const char *path)
{
	if (key->type != keyValueString)
	{
		gEngfuncs.Con_Printf("Expected float value for option %s in %s\n", key->name, path);
		return 0;
	}

	return (float)atof(key->string);
}

static void ParseVector(keyValue_t *key, const char *path, vec_t *dest)
{
	vec3_t temp;

	if (key->type != keyValueString)
	{
		gEngfuncs.Con_Printf("Expected vector value for option %s in %s\n", key->name, path);
		return;
	}

	if (sscanf(key->string, "%f %f %f", &temp[0], &temp[1], &temp[2]) != 3)
		gEngfuncs.Con_Printf("Bad vector value '%s' for option %s in %s\n", key->string, key->name, path);
	else
		VectorCopy(temp, dest);
}

static void ClearConfig(studio_cache_t *cache)
{
	memset(&cache->config, 0, sizeof(cache->config));

	// also clear custom textures
	for (int i = 0; i < cache->numtextures; i++)
	{
		cache->textures[i].diffuse = 0;
	}
}

// this is ok for us because we know the buffer sizes
static void CopyWithoutExtension(char *dst, const char *src)
{
	while (*src && *src != '.')
		*dst++ = *src++;
	*dst = '\0';
}

static void ParseConfig(studio_cache_t *cache, bool flush)
{
	// clear in case we're reloading
	ClearConfig(cache);

	char modelname[64];
	char config_path[128];
	CopyWithoutExtension(modelname, cache->name); // will fit
	sprintf(config_path, "%s.txt", modelname); // will fit

	char *text = (char *)gEngfuncs.COM_LoadFile(config_path, 5, NULL);
	if (!text)
		return;

	keyValue_t root;
	if (!KeyValueParse(&root, text))
	{
		gEngfuncs.Con_Printf("Could not parse file %s\n", config_path);
		KeyValueFree(&root);
		gEngfuncs.COM_FreeFile(text);
		return;
	}

	for (int i = 0; i < root.numSubkeys; i++)
	{
		keyValue_t *subkey = &root.subkeys[i];

		if (!strcmp(subkey->name, "textures"))
		{
			ParseTextures(cache, subkey, flush, config_path);
		}
		else if (!strcmp(subkey->name, "mirror_shell"))
		{
			cache->config.mirror_shell = ParseBoolean(subkey, config_path);
		}
		else if (!strcmp(subkey->name, "mirror_model"))
		{
			cache->config.mirror_model = ParseBoolean(subkey, config_path);
		}
		else if (!strcmp(subkey->name, "origin"))
		{
			ParseVector(subkey, config_path, cache->config.origin);
		}
		else if (!strcmp(subkey->name, "fov_override"))
		{
			cache->config.fov_override = ParseFloat(subkey, config_path);
			if (cache->config.fov_override < 1 || cache->config.fov_override > 179)
			{
				gEngfuncs.Con_Printf("Value out of range for option %s in %s (min 1, max 179)\n", subkey->name, config_path);
				cache->config.fov_override = 0;
			}
		}
		else
		{
			gEngfuncs.Con_Printf("Unrecognized option %s in %s\n", subkey->name, config_path);
		}
	}

	KeyValueFree(&root);
	gEngfuncs.COM_FreeFile(text);
}

static void SetupTextures(studio_cache_t *cache, model_t *model, studiohdr_t *header)
{
	studiohdr_t *textureheader = R_LoadTextures(model, header);
	mstudiotexture_t *textures = (mstudiotexture_t *)((byte *)textureheader + textureheader->textureindex);

	// we need to store numtextures, texture names and config path in the cache so we
	// can reload textures even when we don't have access to a model_t or studiohdr_t

	cache->textures = (mem_texture_t *)Mem_Alloc(sizeof(mem_texture_t) * textureheader->numtextures);
	memset(cache->textures, 0, sizeof(mem_texture_t) * textureheader->numtextures);
	cache->numtextures = textureheader->numtextures;

	// set texture names and check flags
	for (int i = 0; i < textureheader->numtextures; i++)
	{
		mstudiotexture_t *texture = &textures[i];
		mem_texture_t *mem_texture = &cache->textures[i];
		strcpy(mem_texture->name, texture->name); // will fit

		if (texture->flags & STUDIO_NF_FULLBRIGHT)
			cache->needs_renderer = true;
		else if (texture->flags & STUDIO_NF_CHROME && (texture->width != 64 || texture->height != 64))
			cache->needs_renderer = true;
	}
}

static void BuildStudioCache(studio_cache_t *cache, model_t *model, studiohdr_t *header)
{
	strcpy(cache->name, model->name); // will fit
	cache->header_length = header->length;

	if (studio_fastpath)
	{
		// external textures only available with fastpath
		SetupTextures(cache, model, header);
	}

	ParseConfig(cache, false);

	if (studio_fastpath)
	{
		// vbos only used with fastpath (duh)
		BuildStudioVBO(cache, model, header);
	}
}

studio_cache_t *GetStudioCache(model_t *model, studiohdr_t *header)
{
	// see if the cache pointer is in the header
	if (header->id == CACHE_ID && header->version == CACHE_VERSION)
		return &cache_array[header->length];

	// see if this model is cached even
	uint32 name_hash = HashString(model->name);

	studio_cache_t **p = &cache_head;
	for (uint32 h = name_hash; *p; h <<= 2)
	{
		if (!strcmp(model->name, (*p)->name))
		{
			// see if the model has changed (flush command)
			if (header->length != (*p)->header_length)
			{
				// this leaks memory but my computer has a lot of it so i don't care
				memset(*p, 0, sizeof(**p));
				BuildStudioCache(*p, model, header);
			}

			// update the header
			header->id = CACHE_ID;
			header->version = CACHE_VERSION;
			header->length = (int)((*p) - cache_array);
			return *p;
		}

		p = &(*p)->children[h >> 30];
	}

	if (cache_count >= MAX_CACHED)
		Plat_Error("Studio model cache full");

	*p = &cache_array[cache_count++];
	BuildStudioCache(*p, model, header);

	// update the header
	header->id = CACHE_ID;
	header->version = CACHE_VERSION;
	header->length = (int)((*p) - cache_array);
	return *p;
}

studio_cache_t *EntityStudioCache(cl_entity_t *entity)
{
	model_t *model = entity->model;
	if (!model)
		return NULL;

	if (model->type != mod_studio)
		return NULL;

	studiohdr_t *studiohdr = (studiohdr_t *)model->cache.data;
	if (!studiohdr)
		return NULL;

	return GetStudioCache(model, studiohdr);
}

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

	if (studio_fastpath)
		R_StudioCompileShaders();
}

void StudioCacheStats(int *count, int *max)
{
	*count = cache_count;
	*max = MAX_CACHED;
}

void StudioConfigFlush_f(void)
{
	flush_count++;

	for (int i = 0; i < cache_count; i++)
	{
		studio_cache_t *cache = &cache_array[i];
		ParseConfig(cache, true);
	}
}

// not an ideal place for this but ok
studiohdr_t *R_LoadTextures(model_t *model, studiohdr_t *header)
{
	assert(model);

	if (header->textureindex)
		return header;

	model_t *texmodel = (model_t *)model->texinfo;
	if (texmodel && IEngineStudio.Cache_Check(&texmodel->cache))
		return (studiohdr_t *)texmodel->cache.data;

	char path[128];
	strcpy(path, model->name);

	// unsafe but the engine does it too...
	// also lower case for linux??? what the fuck
	strcpy(path + strlen(path) - 4, "t.mdl");

	texmodel = IEngineStudio.Mod_ForName(path, true);
	model->texinfo = (mtexinfo_t *)texmodel;

	// not sure why this is done but ok
	studiohdr_t *textureheader = (studiohdr_t *)texmodel->cache.data;
	strcpy(textureheader->name, path);
	return textureheader;
}
