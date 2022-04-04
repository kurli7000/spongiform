precision mediump float;

uniform sampler2D t_color;
uniform sampler2D t_leiska;

uniform float br;
uniform vec2 contrast;
uniform vec3 biits;
uniform float leiskabr;

varying vec2 tc;

void main() {
	vec3 a, b;
	
	a = texture2D(t_color, tc).rgb*biits;	
	b = texture2D(t_leiska, tc).rgb;
	gl_FragColor = vec4((a*br-contrast.y)*contrast.x+contrast.y-b*leiskabr, 1.0);
}
