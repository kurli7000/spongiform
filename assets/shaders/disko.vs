attribute vec3 a_position;
attribute vec2 a_texcoord;
attribute vec3 a_normal;

uniform mat4 mat;
uniform float pos;

varying vec2 tc;
varying vec2 tc2;

const float pstep = 0.0625;

void main() {
  gl_Position = mat * vec4(a_position, 1.0);
  tc = a_texcoord;
  tc2 = a_texcoord + vec2(pos, pos*3.0) * pstep;
}
