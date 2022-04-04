attribute vec4 a_position;
attribute vec2 a_texcoord;
uniform vec2 dir;

varying vec2 tc0, tc1, tc2, tc3, tc4, tc5, tc6;

void main() {
  gl_Position = a_position;
  tc0 = a_texcoord - dir*5.5;
  tc1 = a_texcoord - dir*3.5;
  tc2 = a_texcoord - dir*1.5;
  tc3 = a_texcoord;
  tc4 = a_texcoord + dir*1.5;
  tc5 = a_texcoord + dir*3.5;
  tc6 = a_texcoord + dir*5.5;
}



