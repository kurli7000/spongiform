attribute vec3 a_position;
attribute vec2 a_texcoord;
attribute vec3 a_normal;
uniform mat4 mat;
varying vec2 tc;
varying vec3 norm;

void main() {
  gl_Position = mat * vec4(a_position, 1.0);
  tc = a_texcoord;
  norm = a_normal;
}
