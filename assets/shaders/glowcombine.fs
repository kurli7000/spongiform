precision mediump float;

uniform sampler2D t_orig;
uniform sampler2D t_blur;

uniform float glowbr;
uniform float fxbr;
uniform float glowdim;

varying vec2 tc;

void main() {
	vec3 a, b, c;
	vec4 col;
	
	a = texture2D(t_orig, tc).rgb;
	b = texture2D(t_blur, tc).rgb;
	col = vec4(a*fxbr + max(b-glowdim, 0.0)*glowbr , 1.0);
	gl_FragColor = col;
}
