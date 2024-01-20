#include "pch.h"

#ifndef SHADER_DIR /* xxd'd shaders */
#include "studio_frag.h"
#include "studio_vert.h"
#endif

typedef struct
{
	int flag;
	const char *define;
} option_info_t;

#define OPTION_INFO(name) { name, "#define " #name "\n" }

static const option_info_t option_info[] =
{
	OPTION_INFO(CAN_MASKED),
	OPTION_INFO(CAN_CHROME),
	OPTION_INFO(CAN_FLATSHADE),

	OPTION_INFO(HAVE_ELIGHTS),
	OPTION_INFO(HAVE_ADDITIVE),
	OPTION_INFO(HAVE_GLOWSHELL),
	OPTION_INFO(HAVE_FOG),
	OPTION_INFO(HAVE_FOG_LINEAR),

	OPTION_INFO(CAN_FULLBRIGHT),
};

static const attribute_t studio_attributes_cpu[] =
{
	{ shader_studio_a_pos, "a_pos" },
	{ shader_studio_a_normal, "a_normal" },
	{ shader_studio_a_texcoord, "a_texcoord" }
};

static const attribute_t studio_attributes_gpu[] =
{
	{ shader_studio_a_pos, "a_pos" },
	{ shader_studio_a_normal, "a_normal" },
	{ shader_studio_a_texcoord, "a_texcoord" },
	{ shader_studio_a_bones, "a_bones" }
};

#define UNIFORM_DEF(name) { Q_OFFSETOF(studio_shader_t, name), #name }

static const uniform_t studio_uniforms[] =
{
	UNIFORM_DEF(u_colormix),

	UNIFORM_DEF(u_chromeorg),
	UNIFORM_DEF(u_chromeright),

	UNIFORM_DEF(u_ambientlight),
	UNIFORM_DEF(u_shadelight),
	UNIFORM_DEF(u_lightvec),

	UNIFORM_DEF(u_lightgamma),
	UNIFORM_DEF(u_brightness),
	UNIFORM_DEF(u_g3),

	UNIFORM_DEF(u_invgamma),

	UNIFORM_DEF(u_texture),

	UNIFORM_DEF(u_tex_flatshade),
	UNIFORM_DEF(u_tex_chrome),
	UNIFORM_DEF(u_tex_fullbright),
	UNIFORM_DEF(u_tex_masked),

	UNIFORM_DEF(u_elight_pos),
	UNIFORM_DEF(u_elight_color)
};

static studio_shader_t studio_shaders[NUM_OPTIONS];

typedef struct
{
	char *data;
	size_t cap;
	size_t len;
} String;

static bool StringAppend(String *dst, const char *src)
{
	if (dst->len == dst->cap)
	{
		// can't copy
		assert(false);
		return false;
	}

	size_t srclen = strlen(src);

	if (srclen >= dst->cap - dst->len)
	{
		// truncation happened, shouldn't happen, if it does no big deal
		assert(false);

		memcpy(dst->data + dst->len, src, dst->cap - dst->len - 1);
		dst->data[dst->cap - 1] = '\0';
		dst->len = dst->cap - 1;
		return false;
	}

	memcpy(dst->data + dst->len, src, srclen + 1);
	dst->len += srclen;
	return true;
}

studio_shader_t *R_StudioSelectShader(int options)
{
	studio_shader_t *dest = &studio_shaders[options];
	if (dest->program)
		return dest;

	char buffer[4096];
	String defines = { buffer, sizeof(buffer), 0 };

	if (studio_gpuskin)
	{
		// we can't use glsl 1.20 because most drivers will syntax error
		// on the std140 ubo. use glsl 1.50 in compat profile if we can,
		// otherwise use glsl 1.20 and hope that it'll work on this card...
		if (GLVersion.major >= 3 && GLVersion.minor >= 2)
			StringAppend(&defines, "#version 150 compatibility\n#define GPU_SKINNING\n");
		else
			StringAppend(&defines, "#version 120\n#define GPU_SKINNING\n");
	}
	else
	{
		StringAppend(&defines, "#version 110\n");
	}

	for (size_t j = 0; j < Q_ARRAYSIZE(option_info); j++)
	{
		if (options & option_info[j].flag)
			StringAppend(&defines, option_info[j].define);
	}

	gEngfuncs.Con_DPrintf("Compiling studio shader with defines:\n%s", defines);

	if (studio_gpuskin)
	{
		LOAD_SHADER(dest, studio, defines.data, defines.len, studio_attributes_gpu, studio_uniforms);
		GLuint block_index = glGetUniformBlockIndex(dest->program, "bones");
		glUniformBlockBinding(dest->program, block_index, 0);
	}
	else
	{
		LOAD_SHADER(dest, studio, defines.data, defines.len, studio_attributes_cpu, studio_uniforms);
	}

	return dest;
}

void R_StudioCompileShaders(void)
{
	int i;

	double end = gEngfuncs.GetAbsoluteTime() + 1;

	for (i = 0; i < NUM_OPTIONS; i++)
	{
		double current = gEngfuncs.GetAbsoluteTime();
		if (current > end)
			break;

		R_StudioSelectShader(i);
	}

	// probably won't do shit but keep it in anyway
	glFlush();
}
