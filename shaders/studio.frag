varying vec2 f_texcoord;
varying vec4 f_color;

#if defined(HAVE_FOG)
varying float f_fog;
#endif

uniform sampler2D u_texture;

#if defined(CAN_MASKED)
uniform bool u_tex_masked;
#endif

void main()
{
	vec4 tex = texture2D(u_texture, f_texcoord);

	// mikkotodo should do this after applying color and fog
	// but that broke everything so i didn't do it yet
#if defined(CAN_MASKED)
	if (u_tex_masked && tex.a < 0.5)
		discard;
#endif

	vec4 result = tex * f_color;

#if defined(HAVE_FOG)
	result.rgb = mix(gl_Fog.color.rgb, result.rgb, f_fog);
#endif

	gl_FragColor = result;
}
