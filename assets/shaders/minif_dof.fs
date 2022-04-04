precision mediump float;

uniform sampler2D t_color;
varying vec2 tc0, tc1, tc2, tc3;

void main() {
	gl_FragColor =
		texture2D(t_color, tc0)*0.25 + 
		texture2D(t_color, tc1)*0.25 + 
		texture2D(t_color, tc2)*0.25 + 
		texture2D(t_color, tc3)*0.25;
}
