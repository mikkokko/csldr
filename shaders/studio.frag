#version 110

varying vec2 f_texcoord;
varying vec4 f_color;
varying float f_fog_frag_coord;

uniform sampler2D u_texture;

uniform bool u_tex_masked;

void main()
{
	// can't use GL_ALPHA_TEST because fuck you
	//gl_FragColor = texture2D(u_texture, f_texcoord) * f_color;

	vec4 tex = texture2D(u_texture, f_texcoord);
	if (u_tex_masked && tex.a < 0.5)
		discard;
		
	vec4 result = tex * f_color;

	// env_fog uses GL_EXP2
	// the underwater fog is GL_LINEAR, won't bother implementing it until someone complains
	float fog = exp(-gl_Fog.density * gl_Fog.density * f_fog_frag_coord * f_fog_frag_coord);
	fog = clamp(fog, 0.0, 1.0);

	result.rgb = mix(gl_Fog.color.rgb, result.rgb, fog);

	gl_FragColor = result;
}
