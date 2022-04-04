precision mediump float;

uniform sampler2D t_color;
varying vec2 tc0;
varying vec2 tc1;
varying float fade;

void main() {
	gl_FragColor = mix(texture2D(t_color, tc0), texture2D(t_color, tc1), fade);
}
