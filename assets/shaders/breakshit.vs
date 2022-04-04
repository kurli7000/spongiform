attribute vec4 a_position;
attribute vec2 a_texcoord;

uniform float time;

varying vec2 tc;
varying float p1, p2;

void main() {
	gl_Position = a_position;
	p1 = sin(time*8.0)*0.02+0.1;
	p2 = sin(time*6.0+p1)*0.05+0.3;
	tc = a_texcoord;
}
