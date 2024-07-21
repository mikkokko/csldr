typedef struct
{
	GLuint location;
	const char *name;
} attribute_t;

typedef struct
{
	size_t offset;
	const char *name;
} uniform_t;

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
	int num_uniforms);

#ifdef SHADER_DIR
#define LOAD_SHADER(obj, name, defs, defs_len, atr, un)	\
obj->program = CreateShaderProgram(#name,				\
	#name ".glsl",										\
	defs,												\
	defs_len,											\
	atr,												\
	Q_ARRAYSIZE(atr),									\
	(byte *)obj,										\
	un,													\
	Q_ARRAYSIZE(un))
#else
#define LOAD_SHADER(obj, name, defs, defs_len, atr, un) \
obj->program = CreateShaderProgram(#name,				\
	(char *)name##_glsl,								\
	(int)name##_glsl_len,								\
	defs,												\
	defs_len,											\
	atr,												\
	Q_ARRAYSIZE(atr),									\
	(byte *)obj,										\
	un,													\
	Q_ARRAYSIZE(un))
#endif
