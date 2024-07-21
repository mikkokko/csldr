#include "pch.h"

#define MAX_SHADER_SOURCE 8192

const char *ShaderType(GLenum type)
{
	switch (type)
	{
	case GL_VERTEX_SHADER:
		return "vertex";

	case GL_FRAGMENT_SHADER:
		return "fragment";
	}

	return "unknown";
}

static GLuint CompileShader(const char *name, const char *defines, int defines_length, const char *base_source, int base_length, GLenum type)
{
	const char *type_string;

	switch (type)
	{
	case GL_VERTEX_SHADER:
		type_string = "#define VERTEX_SHADER\n";
		break;

	case GL_FRAGMENT_SHADER:
		type_string = "#define FRAGMENT_SHADER\n";
		break;

	default:
		assert(false);
		type_string = "";
		break;
	}

	int type_length = strlen(type_string);

	if (defines_length + base_length + type_length > MAX_SHADER_SOURCE)
		Plat_Error("Compiling %s %s shader failed: source is too long\n", name, ShaderType(type));

	int length = 0;
	char source_buffer[MAX_SHADER_SOURCE];

	memcpy(&source_buffer[length], defines, defines_length);
	length += defines_length;

	memcpy(&source_buffer[length], type_string, type_length);
	length += type_length;

	memcpy(&source_buffer[length], base_source, base_length);
	length += base_length;

	const char *source = source_buffer;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, &length);
	glCompileShader(shader);

	GLint is_compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);

	if (!is_compiled)
	{
		char message[1024];
		glGetShaderInfoLog(shader, sizeof(message), NULL, message);
		Plat_Error("Compiling %s %s shader failed:\n%s", name, ShaderType(type), message);
	}

	return shader;
}

#ifdef SHADER_DIR
static char *LoadFromDisk(const char *name, int *psize)
{
	char path[256];
	sprintf(path, SHADER_DIR "/%s", name);

	FILE *f = fopen(path, "rb");
	if (!f)
	{
		Plat_Error("Could not load shader file %s\n", path);
	}
	
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *data = (char *)Mem_TempAlloc(size);
	fread(data, 1, size, f);
	fclose(f);

	*psize = size;
	return data;
}
#endif

GLuint CreateShaderProgram(const char *name,
#ifdef SHADER_DIR
	char *path,
#else
	const char *source,
	int source_length,
#endif
	const char *defines,
	int defines_length,
	const attribute_t *attributes,
	int num_attributes,
	byte *uniform_struct,
	const uniform_t *uniforms,
	int num_uniforms)
{
	int i;

#ifdef SHADER_DIR
	int source_length;
	char *source = LoadFromDisk(path, &source_length);
	GLuint vertex_shader = CompileShader(name, defines, defines_length, source, source_length, GL_VERTEX_SHADER);
	GLuint fragment_shader = CompileShader(name, defines, defines_length, source, source_length, GL_FRAGMENT_SHADER);
	Mem_TempFree(source);
#else
	GLuint vertex_shader = CompileShader(name, defines, defines_length, source, source_length, GL_VERTEX_SHADER);
	GLuint fragment_shader = CompileShader(name, defines, defines_length, source, source_length, GL_FRAGMENT_SHADER);
#endif

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	for (i = 0; i < num_attributes; i++)
		glBindAttribLocation(program, attributes[i].location, attributes[i].name);

	glLinkProgram(program);

	GLint is_linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
	if (!is_linked)
	{
		char message[1024];
		glGetProgramInfoLog(program, sizeof(message), NULL, message);
		Plat_Error("Linking %s program failed:\n%s", name, message);
	}

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);

	// disable for shader cache
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	for (i = 0; i < num_uniforms; i++)
		*((GLint *)(uniform_struct + uniforms[i].offset)) = glGetUniformLocation(program, uniforms[i].name);

	return program;
}
