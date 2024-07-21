// non boolean shader options (needs to match the shader)
#define FOG_MODE_NONE 0
#define FOG_MODE_LINEAR 1
#define FOG_MODE_EXP2 2

#define LIGHTING_MODE_DEFAULT 0
#define LIGHTING_MODE_ADDITIVE 1
#define LIGHTING_MODE_GLOWSHELL 2
#define LIGHTING_MODE_ELIGHTS 3

// shader permutation flags, roughly in order of frequency
typedef struct
{
	unsigned CAN_MASKED;
	unsigned CAN_CHROME;

	unsigned LIGHTING_MODE;
	unsigned FOG_MODE;

	unsigned CAN_FLATSHADE; // who the fuck uses these
	unsigned CAN_FULLBRIGHT; // an "extension" so not too common probably
} studio_options_t;

// vertex attributes
enum
{
	shader_studio_a_pos = 0,
	shader_studio_a_normal = 1,
	shader_studio_a_texcoord = 2,
	shader_studio_a_bones = 3
};

typedef struct studio_shader_s
{
	GLuint program;

	GLint u_bones; // used if ubos are not avaialble

	GLint u_color;

	GLint u_chromeorg;
	GLint u_chromeright;

	GLint u_ambientlight;
	GLint u_lightvec;
	GLint u_shadelight;

	GLint u_lightgamma;
	GLint u_brightness;
	GLint u_invgamma;
	GLint u_g3;

	GLint u_elight_pos;
	GLint u_elight_color;

	GLint u_texture;

	GLint u_tex_flatshade;
	GLint u_tex_chrome;
	GLint u_tex_fullbright;
	GLint u_tex_masked;
} studio_shader_t;

studio_shader_t *R_StudioSelectShader(const studio_options_t *options);

// compiles half of all shader permutations
// called once on startup and once on level load
void R_StudioCompileShaders(void);
