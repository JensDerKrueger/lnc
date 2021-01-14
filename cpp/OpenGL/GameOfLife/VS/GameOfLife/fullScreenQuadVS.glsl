#version 410

in vec3 vPos;
out vec2 tc;

void main() {
  gl_Position = vec4(vPos, 1.0);
  tc = (vPos.xy+vec2(1.0,1.0)) / 2.0;
}
