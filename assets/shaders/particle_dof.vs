attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 mat;
uniform vec3 up, right;
uniform float size;
uniform float focus;
uniform float time;
uniform float grow;

varying vec2 tc0, tc1;
varying float fade;

const float layers = 8.0;
const float maxdof = 7.0/8.0;
const float ramp = 0.125;

void main() {
	float s = (sin(a_normal.z-time*8.0)*0.2+1.0)*size;
	s*=clamp(grow-a_position.z*0.1, 0.0, 1.0);
	gl_Position = mat * vec4(up * a_normal.x * s + right * a_normal.y * s + a_position, 1.0);	
	float dof = clamp(abs(focus-gl_Position.z)*ramp, 0.0, maxdof);
	fade = mod(dof*layers, 1.0);  
	float yoffs = dof*layers-fade; 
	tc0 = vec2(a_texcoord.x, a_texcoord.y/layers + yoffs/layers);
	tc1 = vec2(a_texcoord.x, a_texcoord.y/layers + (yoffs+1.0)/layers);
}
