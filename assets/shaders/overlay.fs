precision mediump float;

uniform sampler2D t_color;
uniform float br;
varying vec2 tc;

void main() {
	gl_FragColor = texture2D(t_color, tc) * br;
}
