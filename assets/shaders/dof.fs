precision mediump float;

uniform sampler2D t_tex1;
uniform sampler2D t_tex2;
uniform sampler2D t_tex3;
uniform sampler2D t_tex4;

varying vec2 tc0, tc1, tc2, tc3;

void main() {
	vec4 t1, t2, t3, t4;
	vec4 col;
	
	t1 = texture2D(t_tex1, tc0);
	t2 = texture2D(t_tex2, tc1);
	t3 = texture2D(t_tex3, tc2);
	t4 = texture2D(t_tex4, tc3);
	
	col = mix(t1, t2, clamp(t2.a * 3.0, 0.0, 1.0));
	col = mix(col, t3, clamp((t3.a - 0.333) * 3.0, 0.0, 1.0));
	col = mix(col, t4, clamp((t4.a - 0.666) * 3.0, 0.0, 1.0));

	gl_FragColor = col;
}
