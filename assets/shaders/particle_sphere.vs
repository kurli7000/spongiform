attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 mat;
uniform vec3 up, right;
uniform float size;
uniform float time;
uniform float bork;

varying vec2 tc;

void main() {
	vec3 pos;
	float rr = sin(a_position.z*45.12-a_position.y+time*2.0)*0.5+0.5;
	float s = size * (1.0-rr*0.666);
	rr = max(rr*12.0, 6.0);

	pos.x = sin(a_position.y*14.87-time*1.75);
	pos.y = sin(a_position.z*17.77+time*1.81);
	pos.z = sin(a_position.x*13.69+time*1.92);
	pos = normalize(pos)*rr;

 	gl_Position = mat * vec4(up * a_normal.x * s + right * a_normal.y * s + pos, 1.0);
	tc = a_texcoord;
}
