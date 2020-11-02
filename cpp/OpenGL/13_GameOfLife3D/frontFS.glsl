#version 410

in vec3 tc;
out vec4 fc;

void main() {
  fc = vec4(tc.x,tc.y,tc.z,1.0);
}
