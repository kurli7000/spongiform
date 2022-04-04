precision mediump float;

uniform sampler2D t_envi;
uniform sampler2D t_diffuse;
uniform vec3 surface;
uniform float br;

varying vec2 tc, tc2;
varying float dist;

void main() {
	float ao = 1.0-texture2D(t_diffuse, tc2).r;
	vec3 envi = texture2D(t_envi, tc).rgb;
	gl_FragColor = vec4(mix(envi, surface, ao)*br, dist);
	//gl_FragColor = vec4(ao,dist);
	//gl_FragColor = vec4(texture2D(t_envi, tc).rgb,   dist);
	//gl_FragColor = vec4(texture2D(t_diffuse, tc2).rgb, dist);
}
