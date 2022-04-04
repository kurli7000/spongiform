precision mediump float;

uniform sampler2D t_color;
uniform sampler2D t_color2;
uniform vec4 br;
uniform float disko;

varying vec2 tc;
varying vec2 tc2;

void main() {
  gl_FragColor = texture2D(t_color, tc)*br + texture2D(t_color2, tc2)*disko*br;
}
