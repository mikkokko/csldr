#include "pch.h"

#define MAX_IMAGES 2048

typedef struct
{
	uint32 hash; // crap
	char *path;
	bool gamma;
	GLuint texture;
	unsigned int flush_count;
} image_t;

static int num_images;
static image_t images[MAX_IMAGES];

// FNV-1a hash (mikkotodo studio_cache dupe)
#define FNV_OFFSET_BASIS32 0x811c9dc5
#define FNV_PRIME32 0x01000193

static uint32 HashString(const char *s)
{
	uint32 hash = FNV_OFFSET_BASIS32;

	for (; *s; s++)
	{
		hash ^= *s;
		hash *= FNV_PRIME32;
	}

	return hash;
}

static image_t *FindImage(uint32 hash)
{
	for (int i = 0; i < num_images; i++)
	{
		image_t *image = &images[i];
		if (image->hash == hash)
			return image;
	}

	return NULL;
}

static GLuint LoadFromFile(char *path, bool gamma, bool flush)
{
	uint32 hash = HashString(path);

	image_t *image = FindImage(hash);
	if (image)
	{
		if (!flush || (image->flush_count == flush_count))
			return image->texture;
	}

	int width, height, comp;
	byte *data = TgaLoad(path, &width, &height, &comp);
	if (!data)
	{
		gEngfuncs.Con_Printf("Could not load %s: %s\n", path, TgaLoadError());
		return 0;
	}

	// gamma correct if wanted
	if (gamma)
	{
		int s = width * height * comp;

		if (comp == 4)
		{
			for (int i = 0; i < s; i += 4)
			{
				data[i+0] = gammavars.textable[data[i+0]];
				data[i+1] = gammavars.textable[data[i+1]];
				data[i+2] = gammavars.textable[data[i+2]];
			}
		}
		else
		{
			for (int i = 0; i < s; i++)
				data[i] = gammavars.textable[data[i]];		
		}
	}

	if (!image)
	{
		if (num_images >= MAX_IMAGES)
			Plat_Error("Studio model texture cache full\n");

		image = &images[num_images++];
		image->hash = hash;
		image->path = Mem_Strdup(path);
		image->gamma = gamma;
		glGenTextures(1, &image->texture);
	}

	image->flush_count = flush_count;

	glPushAttrib(GL_TEXTURE_BIT);

	glBindTexture(GL_TEXTURE_2D, image->texture);

	// mikkotodo handle gl_texturemode?
	GLenum format;

	switch (comp)
	{
	case 1:
		format = GL_LUMINANCE;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		assert(false);
		format = GL_LUMINANCE;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPopAttrib();

	Mem_TempFree(data);
	
	return image->texture;
}

void StudioLoadTexture(keyValue_t *kv, mem_texture_t *dest, bool flush)
{
	for (int i = 0; i < kv->numSubkeys; i++)
	{
		keyValue_t *subkey = &kv->subkeys[i];
		if (subkey->type != keyValueString)
		{
			gEngfuncs.Con_Printf("Subkey %s is not a string, ignoring\n", subkey->name);
			continue;
		}

		// handle the pair
		if (!strcmp(subkey->name, "diffuse"))
		{
			if (!flush && dest->diffuse)
				gEngfuncs.Con_Printf("diffuse specified twice for %s, ignoring the second definition\n", kv->name);
			else
				dest->diffuse = LoadFromFile(subkey->string, true, flush);
		}
		else
		{
			gEngfuncs.Con_Printf("Unrecognized option %s for %s\n", subkey->name, kv->name);
		}
	}
}
