attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 mat;
uniform mat3 nmat;
uniform float focus;
uniform float time;
uniform vec2 wobble;

varying vec2 tc, tc2;
varying float dist;

vec2 envicoord(vec3 u, vec3 n) {
  vec2 pier;
  vec3 r = reflect(u, n);
  float m = 2.0*sqrt(r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));
  pier.x = r.x/m + 0.5;
  pier.y = r.y/m + 0.5;
  return pier;
}

void main() {
  float len = length(a_position);
  float twist = sin(len*2.0+time*16.0);

  vec3 xnorm = a_normal * (1.0 + twist * 0.5 * wobble.y);
  vec3 xpos = a_position * (1.0 + twist * 0.05 * wobble.y);

  vec3 norm = normalize(nmat*xnorm);
  vec4 vpos = mat*vec4(xpos, 1.0);
  vec3 look = -vec3(normalize(vpos));
  dist = (vpos.z - focus * 0.5) / focus;
  tc = envicoord(norm, look); 
  tc2.x = a_texcoord.x;
  tc2.y = 1.0-a_texcoord.y;
  gl_Position = vpos;
}
