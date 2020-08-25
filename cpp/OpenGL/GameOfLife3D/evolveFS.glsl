#version 410

uniform sampler3D gridSampler;
uniform float gridPos;

in vec2 tc;
out float fc;

float evolutionRule(float center, float n);

void main() {
  vec3 texSize = vec3(textureSize(gridSampler,0));
  
  vec2 pixOffset[9] = vec2[]( vec2(-1,-1), vec2(0,-1), vec2(1,-1),
                              vec2(-1, 0),             vec2(1, 0),
                              vec2(-1, 1), vec2(0, 1), vec2(1, 1),
                              vec2(0, 0));
  
  float n = 0;
  for (int i = 0;i<9;i++) {
    vec2 pos2d = tc + pixOffset[i] / texSize.xy;
    n += texture(gridSampler, vec3(pos2d.x, pos2d.y, gridPos-1.0/texSize.z) ).r;
  }
  for (int i = 0;i<8;i++) {
    vec2 pos2d = tc + pixOffset[i] / texSize.xy;
    n += texture(gridSampler, vec3(pos2d.x, pos2d.y, gridPos) ).r;
  }
  for (int i = 0;i<9;i++) {
    vec2 pos2d = tc + pixOffset[i] / texSize.xy;
    n += texture(gridSampler, vec3(pos2d.x, pos2d.y, gridPos+1.0/texSize.z) ).r;
  }

  float gridValue = texture(gridSampler, vec3(tc.x, tc.y, gridPos)).r;

  fc = evolutionRule(gridValue, n);
}
