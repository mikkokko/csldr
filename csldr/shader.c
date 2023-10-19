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
	char source_buffer[MAX_SHADER_SOURCE];

	if (defines_length + base_length > MAX_SHADER_SOURCE)
		Plat_Error("Compiling %s %s shader failed: source is too long\n", name, ShaderType(type));

	memcpy(source_buffer, defines, defines_length);
	memcpy(source_buffer + defines_length, base_source, base_length);

	const char *source = source_buffer;
	int length = defines_length + base_length;

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
	char *vertex_name,
	char *fragment_name,
#else
	const char *vertex_source,
	int vertex_length,
	const char *fragment_source,
	int fragment_length,
#endif
	const char *defines,
	int defines_length,
	const attribute_t *attributes,
	int num_attributes,
	byte *uniform_struct,
	const uniform_t *uniforms,
	int num_uniforms)
{
#ifdef SHADER_DIR
	int vertex_length;
	char *vertex_source = LoadFromDisk(vertex_name, &vertex_length);
	GLuint vertex_shader = CompileShader(name, defines, defines_length, vertex_source, vertex_length, GL_VERTEX_SHADER);
	Mem_TempFree(vertex_source);
#else
	GLuint vertex_shader = CompileShader(name, defines, defines_length, vertex_source, vertex_length, GL_VERTEX_SHADER);
#endif

#ifdef SHADER_DIR
	int fragment_length;
	char *fragment_source = LoadFromDisk(fragment_name, &fragment_length);
	GLuint fragment_shader = CompileShader(name, defines, defines_length, fragment_source, fragment_length, GL_FRAGMENT_SHADER);
	Mem_TempFree(fragment_source);
#else
	GLuint fragment_shader = CompileShader(name, defines, defines_length, fragment_source, fragment_length, GL_FRAGMENT_SHADER);
#endif

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	for (int i = 0; i < num_attributes; i++)
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

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	for (int i = 0; i < num_uniforms; i++)
		*((GLint *)(uniform_struct + uniforms[i].offset)) = glGetUniformLocation(program, uniforms[i].name);

	return program;
}
