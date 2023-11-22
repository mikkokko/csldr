// shader permutation flags, roughly in order of frequency
enum
{
	CAN_MASKED = (1 << 0),
	CAN_CHROME = (1 << 1),

	HAVE_ELIGHTS = (1 << 2),
	HAVE_ADDITIVE = (1 << 3),
	HAVE_GLOWSHELL = (1 << 4),
	HAVE_FOG = (1 << 5),
	HAVE_FOG_LINEAR = (1 << 6),

	CAN_FLATSHADE = (1 << 7), // who the fuck uses these
	CAN_FULLBRIGHT = (1 << 8), // an "extension" so not too common probably

	NUM_OPTIONS = (1 << 9)
};

// vertex attributes
enum
{
	shader_studio_a_pos = 0,
	shader_studio_a_normal = 1,
	shader_studio_a_texcoord = 2,

	// only for gpu skinning
	shader_studio_a_bones = 3
};

typedef struct studio_shader_s
{
	GLuint program;

	GLint u_colormix;

	GLint u_chromeorg;
	GLint u_chromeright;

	GLint u_ambientlight;
	GLint u_lightvec;
	GLint u_shadelight;

	GLint u_lightgamma;
	GLint u_brightness;
	GLint u_invgamma;
	GLint u_g3;

	GLint u_texture;

	GLint u_tex_flatshade;
	GLint u_tex_chrome;
	GLint u_tex_fullbright;
	GLint u_tex_masked;

	GLint u_elight_pos;
	GLint u_elight_color;
} studio_shader_t;

studio_shader_t *R_StudioSelectShader(int options);

// compiles as many shader combos as it can in 0.5 seconds
void R_StudioCompileShaders(void);
