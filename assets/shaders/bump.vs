attribute vec4 a_position;
attribute vec2 a_texcoord;
varying vec2 tc0, tc1, tc2;

uniform vec2 texel;

void main() {
  gl_Position = a_position;
  tc0 = a_texcoord;
  tc1 = a_texcoord + vec2(texel.x, 0.0);
  tc2 = a_texcoord + vec2(0.0, texel.y);
}
