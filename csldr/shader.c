#include "pch.h"

static GLuint CompileShader(const char *name, const char *source, int length, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, &length);
	glCompileShader(shader);

	GLint is_compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);

	if (!is_compiled)
	{
		char message[1024];
		glGetShaderInfoLog(shader, sizeof(message), NULL, message);
		Plat_Error("Compiling %s shader failed:\n%s", name, message);
	}

	return shader;
}

GLuint CreateShaderProgram(const char *name,
#ifdef SHADERS_FROM_DISK
	const char *vertex_path,
	const char *fragment_path,
#else
	const char *vertex_source,
	int vertex_length,
	const char *fragment_source,
	int fragment_length,
#endif
	const attribute_t *attributes,
	int num_attributes,
	const uniform_t *uniforms,
	int num_uniforms)
{
#ifdef SHADERS_FROM_DISK
	// usehunk 2 = Hunk_TempAlloc
	int vertex_length;
	char *vertex_source = (char *)gEngfuncs.COM_LoadFile(vertex_path, 2, &vertex_length);
	if (!vertex_source)
		Plat_Error("Could not load shader file %s\n", vertex_path);
#endif

	GLuint vertex_shader = CompileShader(name, vertex_source, vertex_length, GL_VERTEX_SHADER);

#ifdef SHADERS_FROM_DISK
	int fragment_length;
	char *fragment_source = (char *)gEngfuncs.COM_LoadFile(fragment_path, 2, &fragment_length);
	if (!fragment_source)
		Plat_Error("Could not load shader file %s\n", fragment_path);
#endif

	GLuint fragment_shader = CompileShader(name, fragment_source, fragment_length, GL_FRAGMENT_SHADER);

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
		*(uniforms[i].location) = glGetUniformLocation(program, uniforms[i].name);

	return program;
}
