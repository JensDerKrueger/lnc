#version 410

uniform mat4 MVP;
in vec3 vPos;
out vec3 tc;

void main() {
  gl_Position = MVP * vec4(vPos, 1.0);
  tc = vPos+0.5;
}
