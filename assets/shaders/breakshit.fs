precision mediump float;

uniform sampler2D t_color;
uniform float param;
uniform float br;

varying vec2 tc;
varying float p1, p2;

void main() {
	vec2 texc = tc;
	mediump float a = mod(tc.x, p1 * 2.0);
	mediump float b = mod(tc.x, p1);
	mediump float c = mod(tc.x, p2);
	mediump float d = mod(tc.x, p2 * 0.5);
	texc.x += step(a, p1) * 0.3 * param - step(b, p1 - 0.02) * 0.1 * param; 
	texc.y += step(c, p2 - 0.01) * 0.1 * param - step(d, p2 * 0.4) * 0.2 * param; 
	gl_FragColor = texture2D(t_color, texc)*br;
}
