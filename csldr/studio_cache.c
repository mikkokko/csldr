#include "pch.h"

#define CACHE_ID (('c' << 0) + ('s' << 8) + ('l' << 16) + ('d' << 24))
#define CACHE_VERSION (('r' << 0) + ('m' << 8) + ('d' << 16) + ('l' << 24))

#define MAX_CACHED 4096

static int cache_count;
static studio_cache_t cache_array[MAX_CACHED];
static studio_cache_t *cache_head;

unsigned int flush_count;

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

		if (!strcmp(subkey->name, "mirror_shell"))
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

static void BuildStudioCache(studio_cache_t *cache, model_t *model, studiohdr_t *header)
{
	strcpy(cache->name, model->name); // will fit
	cache->header_length = header->length;
	ParseConfig(cache, false);
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
