attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 mat;
uniform vec3 up, right;
uniform float size;
uniform float time;
uniform vec3 center;

varying vec2 tc;

void main() {
	vec3 pos;
	float rr = sin(a_position.z*45.12-a_position.y+time*1.0)*3.0;
	rr*=rr*rr;
	vec2 pier;

	pos.x = sin(a_position.y*14.87-time*0.75);
	pos.y = sin(a_position.z*17.77+time*0.81);
	pos.z = sin(a_position.x*13.69+time*0.92);
	pos = normalize(pos)*rr + center;

 	gl_Position = mat * vec4(up * a_normal.x * size + right * a_normal.y * size+ pos, 1.0);
	tc = a_texcoord;
}
