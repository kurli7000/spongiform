precision mediump float;

uniform sampler2D t_color;

varying vec2 tc0, tc1, tc2, tc3, tc4, tc5;

void main() {
	vec4 pier = 
		texture2D(t_color, tc0)*0.10+
		texture2D(t_color, tc1)*0.20+
		texture2D(t_color, tc2)*0.20+
		texture2D(t_color, tc3)*0.20+
		texture2D(t_color, tc4)*0.20+
		texture2D(t_color, tc5)*0.10;
	gl_FragColor = pier;
}




