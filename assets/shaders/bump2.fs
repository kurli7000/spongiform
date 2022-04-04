precision mediump float;

varying vec2 tc0, tc1, tc2;

uniform sampler2D t_color;
uniform float br;
uniform float powah;

void main() {
	vec4 t0 = texture2D(t_color, tc0)*br;
	vec4 t1 = texture2D(t_color, tc1)*br;
	vec4 t2 = texture2D(t_color, tc2)*br;

	vec2 tc;
	tc.x = tc0.x + (t0.r - t1.r) * 2.0 * powah;
	tc.y = tc0.y + (t0.r - t2.r) * 2.0 * powah;

	gl_FragColor = texture2D(t_color, tc)*br + t0;
}
