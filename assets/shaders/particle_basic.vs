attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 mat;
uniform vec3 up, right;
uniform float size;

varying vec2 tc;

void main() {
 	gl_Position = mat * vec4(up * a_normal.x * size + right * a_normal.y * size + a_position, 1.0);
	tc = a_texcoord;
}
