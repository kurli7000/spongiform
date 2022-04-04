precision mediump float;

uniform sampler2D t_color;

varying vec2 tc;

void main() {
	gl_FragColor = texture2D(t_color, tc) * 0.5;
}
