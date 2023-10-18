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
	int num_uniforms);

#ifdef SHADER_DIR
#define LOAD_SHADER(obj, name, defs, defs_len, atr, un)	\
obj->program = CreateShaderProgram(#name,				\
	#name ".vert",										\
	#name ".frag",										\
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
	(char *)name##_vert,								\
	(int)name##_vert_len,								\
	(char *)name##_frag,								\
	(int)name##_frag_len,								\
	defs,												\
	defs_len,											\
	atr,												\
	Q_ARRAYSIZE(atr),									\
	(byte *)obj,										\
	un,													\
	Q_ARRAYSIZE(un))
#endif
