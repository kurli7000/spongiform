attribute vec3 a_position;
attribute vec3 a_normal;

uniform mat4 mat;
uniform mat3 nmat;
uniform float focus;

varying vec3 normal;
varying vec3 look;
varying vec3 pos;
varying float dist;

void main() {
  vec4 vpos = mat*vec4(a_position, 1.0);
  normal = normalize(nmat*a_normal);
  pos = a_normal * 32.0 + a_position * 0.5;
  look = vec3(normalize(vpos));
  dist = 1.0 - (vpos.z - focus * 0.5) / focus;
  gl_Position = vpos;
}
