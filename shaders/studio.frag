#version 110

varying vec2 f_texcoord;
varying vec4 f_color;
varying float f_fog;

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

	result.rgb = mix(gl_Fog.color.rgb, result.rgb, f_fog);

	gl_FragColor = result;
}
