precision mediump float;

uniform sampler2D t_color;
uniform vec4 br;

varying vec2 tc;
varying vec3 norm;

void main() {
  gl_FragColor = texture2D(t_color, tc)*br;
}
