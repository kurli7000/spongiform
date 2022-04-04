attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 mat;
uniform vec3 up, right;
uniform float size;
uniform float time;
uniform float bork;
uniform vec3 wobble;
uniform vec2 middle;

varying vec2 tc;

void main() {
	vec3 pos;
	float r = 6.0
		+ sin(a_position.x*5.0*wobble.x+time*14.0 
		- sin(a_position.y*6.0*wobble.x-time*16.0+a_position.x*4.0))*wobble.y 
		+ sin(a_position.x*10.0+time*512.0)*wobble.z*0.35;
	
	float a = a_position.x;
	float b = a_position.y;
	pos.x = sin(a)*sin(b);
	pos.y = cos(b);
	pos.z = cos(a)*sin(b);
	pos = normalize(pos)*r;

	vec4 posss = mat * vec4(up * a_normal.x * size + right * a_normal.y * size + pos, 1.0);
 	posss.xy += middle * posss.w;
 	gl_Position = posss;

	tc = a_texcoord;
}
