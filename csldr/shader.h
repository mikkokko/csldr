typedef struct
{
	GLuint location;
	const char *name;
} attribute_t;

typedef struct
{
	GLint *location;
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
	const attribute_t *attributes,
	int num_attributes,
	const uniform_t *uniforms,
	int num_uniforms);

#ifdef SHADER_DIR
#define LOAD_SHADER(name, vs, fs, atr, un)			\
shader_##name.program = CreateShaderProgram(#name,	\
	#vs ".vert",							        \
	#fs ".frag",							        \
	atr,								            \
	Q_ARRAYSIZE(atr),					            \
	un,								                \
	Q_ARRAYSIZE(un))
#else
#define LOAD_SHADER(name, vs, fs, atr, un)			\
shader_##name.program = CreateShaderProgram(#name,	\
	(char *)vs##_vert,					            \
	(int)vs##_vert_len,					            \
	(char *)fs##_frag,					            \
	(int)fs##_frag_len,					            \
	atr,								            \
	Q_ARRAYSIZE(atr),					            \
	un,								                \
	Q_ARRAYSIZE(un))
#endif
