precision mediump float;

uniform sampler2D t_color;

varying vec2 tc0, tc1, tc2, tc3, tc4, tc5, tc6;

void main() {
	vec4 pier = 
		texture2D(t_color, tc0)*0.1+
		texture2D(t_color, tc1)*0.15+
		texture2D(t_color, tc2)*0.2+
		texture2D(t_color, tc3)*0.1+
		texture2D(t_color, tc4)*0.2+
		texture2D(t_color, tc5)*0.15+
		texture2D(t_color, tc6)*0.1;
	gl_FragColor = pier;
}

