attribute vec4 a_position;
attribute vec2 a_texcoord;

uniform float zoomfactor;
uniform vec2 center;

varying vec2 tc0, tc1, tc2, tc3, tc4, tc5;

void main() {
  gl_Position = a_position;
  tc0 = a_texcoord;
  tc1 = (a_texcoord-center)*(1.0-zoomfactor*1.0)+center;
  tc2 = (a_texcoord-center)*(1.0-zoomfactor*2.0)+center;
  tc3 = (a_texcoord-center)*(1.0-zoomfactor*3.0)+center;
  tc4 = (a_texcoord-center)*(1.0-zoomfactor*4.0)+center;
  tc5 = (a_texcoord-center)*(1.0-zoomfactor*5.0)+center;
}
