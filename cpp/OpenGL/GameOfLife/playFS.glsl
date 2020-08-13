#version 410

uniform sampler2D gridSampler;

in vec2 tc;
out vec4 FragColor;

void main() {
  float gridValue = texture(gridSampler, tc).r;
  FragColor = vec4(gridValue, gridValue, gridValue, 1.0);
}
