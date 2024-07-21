#line 1

#if UBO_ABLE
#extension GL_ARB_uniform_buffer_object : require
#endif

//------------------------------------------------
// shared
//------------------------------------------------

varying vec2 f_texcoord;
varying vec4 f_color;

#if FOG_MODE
varying float f_fog;
#endif

//------------------------------------------------
// vertex shader
//------------------------------------------------

#if defined(VERTEX_SHADER)

// non boolean shader options (needs to match the code)
#define FOG_MODE_NONE 0
#define FOG_MODE_LINEAR 1
#define FOG_MODE_EXP2 2

#define LIGHTING_MODE_NORMAL 0
#define LIGHTING_MODE_ADDITIVE 1
#define LIGHTING_MODE_GLOWSHELL 2
#define LIGHTING_MODE_ELIGHTS 3

attribute vec3 a_pos;
attribute vec3 a_normal;
attribute vec2 a_texcoord;
attribute vec2 a_bones;

#if UBO_ABLE
layout (std140) uniform bones
{
	mat3x4 u_bones[128];
};
#else
uniform mat3x4 u_bones[128];
#endif

uniform vec4 u_color; // 4th component is the scale with glowshell

#if CAN_CHROME
uniform vec3 u_chromeorg;
uniform vec3 u_chromeright;
#endif

#if (LIGHTING_MODE == LIGHTING_MODE_NORMAL) || (LIGHTING_MODE == LIGHTING_MODE_ELIGHTS)
uniform float u_ambientlight;
uniform float u_shadelight;
uniform vec3 u_lightvec;

uniform float u_lightgamma;
uniform float u_brightness;
uniform float u_g3;
uniform float u_invgamma;
#endif

#if (LIGHTING_MODE == LIGHTING_MODE_ELIGHTS)
#define MAX_ELIGHTS 3
uniform vec4 u_elight_pos[MAX_ELIGHTS]; // 4th component stores radius*radius
uniform vec3 u_elight_color[MAX_ELIGHTS];
#endif

#if CAN_FLATSHADE
uniform bool u_tex_flatshade;
#endif

#if CAN_CHROME
uniform bool u_tex_chrome;
#endif

#if CAN_FULLBRIGHT
uniform bool u_tex_fullbright;
#endif

#if (LIGHTING_MODE == LIGHTING_MODE_NORMAL) || (LIGHTING_MODE == LIGHTING_MODE_ELIGHTS)
float gamma_correct_light(float value)
{
	float f = pow(value, u_lightgamma) * max(u_brightness, 1.0);

	if (f > u_g3)
		f = (f - u_g3) / (1.0 - u_g3) * 0.875 + 0.125;
	else
		f = (f / u_g3) * 0.125;

	return clamp(pow(f, u_invgamma), 0.0, 1.0);
}

#if (LIGHTING_MODE == LIGHTING_MODE_ELIGHTS)
vec3 add_elights(vec3 color, vec3 elight)
{
	vec3 invgamma = vec3(u_invgamma);
	vec3 gamma = vec3(1.0 / u_invgamma);

	vec3 result = pow((pow(color, gamma) + elight), invgamma);
	return min(result, 1.0);
}
#endif

vec4 vertex_color(vec3 pos_anim, vec3 normal_anim)
{
#if CAN_FULLBRIGHT
	if (u_tex_fullbright)
		return vec4(vec3(1.0), u_color.a);
#endif
	
	// attempts to mimic engine's R_StudioLighting
	float diffuse;

#if CAN_FLATSHADE
	if (u_tex_flatshade)
	{
		diffuse = 0.8;
	}
	else
#endif
	{					
		// engine's v_lambert1, doesn't change
		const float lambert = 1.4953241;

		float ndl = dot(normal_anim, u_lightvec);
		
		 // assumes that lambert >= 1.0
		diffuse = (1.0 - ndl) * (1.0 / lambert);
		diffuse = min(diffuse, 1.0);
	}

	float result = u_ambientlight + (u_shadelight * diffuse);
	result = clamp(result, 0.0, 1.0);
	result = gamma_correct_light(result);

	vec4 my_color = u_color;
	my_color.rgb *= result;

	// add elights, if any
#if (LIGHTING_MODE == LIGHTING_MODE_ELIGHTS)
	vec3 elights = vec3(0.0);

	for (int i = 0; i < MAX_ELIGHTS; i++)
	{
		vec3 dir = u_elight_pos[i].xyz - pos_anim;
		float NdotL = max(dot(normal_anim, dir), 0.0); // don't normalize!!!

		// wtf is this attenuation
		float mag_squared = dot(dir, dir);
		float rad_squared = u_elight_pos[i].w;

		float atten = rad_squared / (mag_squared * sqrt(mag_squared));

		elights += u_elight_color[i] * NdotL * atten;
	}

	my_color.rgb = add_elights(my_color.rgb, elights);
#endif

	return my_color;
}
#endif

void main()
{
	mat3x4 bone = u_bones[int(a_bones.x)];
	mat3x4 bone_normal = u_bones[int(a_bones.y)];

	vec3 pos_anim = vec4(a_pos, 1.0) * bone;
	vec3 normal_anim = a_normal * mat3(bone_normal);

	normal_anim = normalize(normal_anim);

#if CAN_CHROME
	if (u_tex_chrome)
	{
		// attempts to mimic engine's R_StudioChrome
		vec3 dir = normalize(vec3(
			bone_normal[0][3] - u_chromeorg[0],
			bone_normal[1][3] - u_chromeorg[1],
			bone_normal[2][3] - u_chromeorg[2]));

		vec3 up = normalize(cross(dir, u_chromeright));
		vec3 side = normalize(cross(dir, up));

		mat3 mat = transpose(mat3(bone_normal));
		vec3 up_anim = up * mat;
		vec3 side_anim = side * mat;

		// mikkotodo why the fuck this this needed???
		f_texcoord.x = 1.0 - (dot(a_normal, side_anim) + 1.0) * 0.5;
		f_texcoord.y = (dot(a_normal, up_anim) + 1.0) * 0.5;
	}
	else
#endif
	{
		f_texcoord = a_texcoord;
	}

#if (LIGHTING_MODE == LIGHTING_MODE_GLOWSHELL)
	pos_anim += normal_anim * u_color.a;
	f_color = vec4(u_color.rgb, 1.0);
	f_texcoord *= (1.0 / 8.0); // fudge the texcoords
#elif (LIGHTING_MODE == LIGHTING_MODE_ADDITIVE)
	f_color = vec4(vec3(u_color.a), 1.0);
#else
	f_color = vertex_color(pos_anim, normal_anim);
#endif

	gl_Position = gl_ModelViewProjectionMatrix * vec4(pos_anim, 1.0);

	// do fog in the vertex shader until someone complains
#if FOG_MODE
	// wtf is gl_FogCoord
	float fog_coord = gl_Position.w;

	#if (FOG_MODE == FOG_MODE_LINEAR)
		f_fog = (gl_Fog.end - fog_coord) * gl_Fog.scale;
	#else
		f_fog = exp(-gl_Fog.density * gl_Fog.density * fog_coord * fog_coord);
	#endif

	f_fog = clamp(f_fog, 0.0, 1.0);
#endif
}

#endif

//------------------------------------------------
// fragment shader
//------------------------------------------------

#if defined(FRAGMENT_SHADER)

uniform sampler2D u_texture;

#if CAN_MASKED
uniform bool u_tex_masked;
#endif

void main()
{
	vec4 tex = texture2D(u_texture, f_texcoord);
#if CAN_MASKED
	if (u_tex_masked && tex.a < 0.5)
		discard;
#endif

	vec4 result = tex * f_color;

#if FOG_MODE
	result.rgb = mix(gl_Fog.color.rgb, result.rgb, f_fog);
#endif

	gl_FragColor = result;
}

#endif
