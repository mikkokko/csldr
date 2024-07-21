#include "pch.h"

#ifndef SHADER_DIR /* xxd'd shaders */
#include "studio_glsl.h"
#endif

typedef struct
{
	int offset;
	unsigned max;
	const char *define_fmt;
} option_info_t;

#define OPTION_INFO(type, field, max) { offsetof(type, field), max, "#define " #field " %u\n" }

static const option_info_t option_info[] =
{
	OPTION_INFO(studio_options_t, CAN_MASKED, 1),
	OPTION_INFO(studio_options_t, CAN_CHROME, 1),

	OPTION_INFO(studio_options_t, LIGHTING_MODE, 3),
	OPTION_INFO(studio_options_t, FOG_MODE, 2),

	OPTION_INFO(studio_options_t, CAN_FLATSHADE, 1),
	OPTION_INFO(studio_options_t, CAN_FULLBRIGHT, 1),
};

// no constexpr, needs to be manually updated
// ValidatePermutationCount checks if it's off
#define NUM_PERMUTATIONS 192

static const attribute_t studio_attributes[] =
{
	{ shader_studio_a_pos, "a_pos" },
	{ shader_studio_a_normal, "a_normal" },
	{ shader_studio_a_texcoord, "a_texcoord" },
	{ shader_studio_a_bones, "a_bones" }
};

#define UNIFORM_DEF(name) { Q_OFFSETOF(studio_shader_t, name), #name }

static const uniform_t studio_uniforms[] =
{
	UNIFORM_DEF(u_bones),

	UNIFORM_DEF(u_color),

	UNIFORM_DEF(u_chromeorg),
	UNIFORM_DEF(u_chromeright),

	UNIFORM_DEF(u_ambientlight),
	UNIFORM_DEF(u_shadelight),
	UNIFORM_DEF(u_lightvec),

	UNIFORM_DEF(u_lightgamma),
	UNIFORM_DEF(u_brightness),
	UNIFORM_DEF(u_invgamma),
	UNIFORM_DEF(u_g3),

	UNIFORM_DEF(u_elight_pos),
	UNIFORM_DEF(u_elight_color),

	UNIFORM_DEF(u_texture),

	UNIFORM_DEF(u_tex_flatshade),
	UNIFORM_DEF(u_tex_chrome),
	UNIFORM_DEF(u_tex_fullbright),
	UNIFORM_DEF(u_tex_masked)
};

static studio_shader_t studio_shaders[NUM_PERMUTATIONS];

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
		// truncated
		assert(false);
		return false;
	}

	memcpy(dst->data + dst->len, src, srclen + 1);
	dst->len += srclen;
	return true;
}

static bool StringAppendf(String *dst, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int result = vsnprintf(dst->data + dst->len, dst->cap - dst->len, fmt, ap);
	va_end(ap);

	if (result < 0)
	{
		// wtf
		assert(false);
		return false;
	}

	if ((unsigned)result >= dst->cap - dst->len)
	{
		// truncated
		assert(false);
		return false;
	}

	// i guess
	dst->len += result;
	return true;
}

#define OPTION_VALUE(option, object) *((unsigned *)((byte *)(object) + (option)->offset))

inline static unsigned OptionValue(const option_info_t *option, const void *object)
{
	unsigned value = *((unsigned *)((byte *)object + option->offset));
	assert(value <= option->max);
	return value;
}

#ifndef NDEBUG
static void ValidatePermutationCount(void)
{
	unsigned index = 0;
	unsigned accum = 1;

	for (size_t i = 0; i < Q_ARRAYSIZE(option_info); i++)
	{
		const option_info_t *option = &option_info[i];
		index += (accum * option->max);
		accum *= (option->max + 1);
	}

	assert((index + 1) == NUM_PERMUTATIONS);
}
#endif

static unsigned ShaderIndex(const studio_options_t *options)
{
	unsigned index = 0;
	unsigned accum = 1;

	for (size_t i = 0; i < Q_ARRAYSIZE(option_info); i++)
	{
		const option_info_t *option = &option_info[i];
		index += (accum * OptionValue(option, options));
		accum *= (option->max + 1);
	}

	assert(index < NUM_PERMUTATIONS);
	return index;
}

studio_shader_t *R_StudioSelectShaderIndex(unsigned index)
{
	studio_shader_t *shader = &studio_shaders[index];
	if (shader->program)
		return shader;

	char buffer[4096];
	String defines = { buffer, sizeof(buffer), 0 };

	if (studio_uboable)
	{
		// we can't use glsl 1.20 because most drivers will syntax error
		// on the std140 ubo. use glsl 1.50 in compat profile if we can,
		// otherwise use glsl 1.20 and hope that it'll work on this card...
		if (GLVersion.major >= 3 && GLVersion.minor >= 2)
			StringAppend(&defines, "#version 150 compatibility\n#define UBO_ABLE 1\n");
		else
			StringAppend(&defines, "#version 120\n#define UBO_ABLE 1\n");
	}
	else
		StringAppend(&defines, "#version 120\n");

	unsigned j = index;

	for (size_t i = 0; i < Q_ARRAYSIZE(option_info); i++)
	{
		const option_info_t *option = &option_info[i];

		unsigned count = option->max + 1;
		unsigned value = (j % count);
		j = (j / count);

		StringAppendf(&defines, option->define_fmt, value);
	}

	gEngfuncs.Con_DPrintf("Compiling studio shader with defines:\n%s", defines);

	LOAD_SHADER(shader, studio, defines.data, defines.len, studio_attributes, studio_uniforms);

	if (studio_uboable)
	{
		GLuint block_index = glGetUniformBlockIndex(shader->program, "bones");
		glUniformBlockBinding(shader->program, block_index, 0);
	}

	// set uniforms that never change
	glUniform1i(shader->u_texture, 0);

	return shader;
}

studio_shader_t *R_StudioSelectShader(const studio_options_t *options)
{
	unsigned index = ShaderIndex(options);
	return R_StudioSelectShaderIndex(index);
}

void R_StudioCompileShaders(void)
{
	static int current = 0;

	if (current == NUM_PERMUTATIONS)
		return;

#ifndef NDEBUG
	ValidatePermutationCount();
#endif

	int end = current ? NUM_PERMUTATIONS : (NUM_PERMUTATIONS / 2);

	for (; current < end; current++)
		R_StudioSelectShaderIndex(current);

	// probably won't do shit but keep it in anyway
	glFlush();
}
