attribute vec4 a_position;
attribute vec2 a_texcoord;

varying vec2 tc0, tc1, tc2, tc3;

uniform float bork;

void main() {
  gl_Position = a_position;
  tc0 = a_texcoord + vec2( 0.1,  0.1) * bork;
  tc1 = a_texcoord + vec2(-0.1,  0.1) * bork;
  tc2 = a_texcoord + vec2( 0.1, -0.1) * bork;
  tc3 = a_texcoord + vec2(-0.1, -0.1) * bork;
}
