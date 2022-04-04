attribute vec4 a_position;
attribute vec2 a_texcoord;
varying vec2 tc;

void main() {
  gl_Position = a_position;
  tc = a_texcoord;
}
