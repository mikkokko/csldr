#include "pch.h"

#define MAX_IMAGES 2048

typedef struct image_s
{
	struct image_s *children[4];

	char *path;
	bool gamma;
	GLuint texture;
	unsigned int flush_count;
} image_t;

static int image_count;
static image_t images_array[MAX_IMAGES];
static image_t *images_head;

static image_t *FindImage(const char *path)
{
	image_t **p = &images_head;

	for (uint32 h = HashString(path); *p; h <<= 2)
	{
		if (!strcmp((*p)->path, path))
			return *p;

		p = &(*p)->children[h >> 30];
	}

	if (image_count >= MAX_IMAGES)
		Plat_Error("Studio model texture cache full\n");

	*p = &images_array[image_count++];
	return *p;
}

static GLuint LoadFromFile(char *path, bool gamma, bool flush)
{
	image_t *image = FindImage(path);
	if (image->texture)
	{
		if (!flush || (image->flush_count == flush_count))
			return image->texture;
	}

	int width, height, comp;
	byte *data = TgaLoad(path, &width, &height, &comp);
	if (!data)
	{
		// do a fatal error for now, TgaLoad leaks memory on failure
		// and we don't handle failures properly here...
		Plat_Error("Could not load %s: %s\n", path, TgaLoadError());
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

	if (!image->texture)
	{
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
