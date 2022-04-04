precision mediump float;

varying vec2 tc0, tc1, tc2;

uniform sampler2D t_color;
uniform sampler2D t_envi;
uniform float time;
uniform float powah;
uniform float br;


void main() {
	vec4 t0 = texture2D(t_color, tc0);
	vec4 t1 = texture2D(t_color, tc1);
	vec4 t2 = texture2D(t_color, tc2);

	vec2 tc;
	tc.x = 0.3 + tc0.x * 0.4 + (t0.r - t1.r) * powah - time * 0.1;
	tc.y = 0.3 + tc0.y * 0.4 + (t0.r - t2.r) * powah;

	gl_FragColor = texture2D(t_envi, tc) * br;
}
