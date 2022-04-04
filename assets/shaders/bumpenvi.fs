precision mediump float;

uniform sampler2D t_envi;
uniform vec3 wobble;
uniform float br;

varying vec3 normal;
varying vec3 look;
varying vec3 pos;
varying float dist;

vec2 envicoord(vec3 u, vec3 n) {
	vec2 pier;
	vec3 r = reflect(u, n);
	float m = 2.0*sqrt(r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));
	pier.x = r.x/m + 0.5;
	pier.y = r.y/m + 0.5;
	return pier;
}

void main() {
	vec3 norm = normal;
	vec3 bump;
	vec3 pos2 = pos * wobble.x;

	pos2.x += wobble.z;

	bump.x = sin(pos2.x * 2.17 - sin(pos2.z * 7.791 + sin(pos2.y * 12.24)*0.25)*0.5) * 0.1;
	bump.y = sin(pos2.y * 3.19 + sin(pos2.x * 6.812 + sin(pos2.z * 14.18)*0.25)*0.5) * 0.1;
	bump.z = sin(pos2.z * 2.75 - sin(pos2.y * 5.468 - sin(pos2.x * 15.27)*0.25)*0.5) * 0.1;

	vec2 tc = envicoord(look, normalize(norm+bump*wobble.y));
	float brr = (0.5+length(bump)*5.0)*br;
	gl_FragColor = vec4(texture2D(t_envi, tc).rgb*brr, dist);
	
}
