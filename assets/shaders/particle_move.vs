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
	float s = size * 1.5;

	pos = a_position * 6.0;
	pos.x += sin(a_position.y*34.87-time*14.75)*0.2;
	pos.y += sin(a_position.z*57.77+time*15.81)*0.2;
	pos.z += sin(a_position.x*43.69+time*16.92)*0.2;

 	gl_Position = mat * vec4(up * a_normal.x * s + right * a_normal.y * s + pos, 1.0);
	tc = a_texcoord;
}
