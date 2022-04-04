precision mediump float;

uniform sampler2D t_orig;
uniform sampler2D t_blur;
uniform float magic;
uniform float glowbr;
uniform float fxbr;
varying vec2 tc;

void main() {
	vec3 a, b;
	
	a = texture2D(t_orig, tc).rgb;
	b = texture2D(t_blur, tc).rgb;
	
	gl_FragColor = vec4(a+(a-b)*magic + b*b*glowbr, 1.0)*fxbr;
}
