#version 410

uniform sampler3D noise;
uniform sampler2D frontFaces;
uniform sampler2D backFaces;
uniform vec2 cursorPos;
uniform float cursorDepth;
uniform float brushSize;
uniform float brushDensity;
uniform int paintState;

vec3 computeCursorVolumePos() {
  vec3 cursorEntry = texture(frontFaces, cursorPos).xyz;
  vec3 cursorExit  = texture(backFaces, cursorPos).xyz;
  return cursorEntry + cursorDepth * (cursorExit-cursorEntry);
}

uniform sampler3D gridSampler;
uniform float gridPos;

in vec2 tc;
out float fc;


int evolutionRule(int center, int n);

void main() {
  vec3 texSize = vec3(textureSize(gridSampler,0));
  vec3 posInVolume = vec3(tc.x, tc.y, gridPos);
  float gridValue = texture(gridSampler, posInVolume).r;

  if (paintState == 0) {
    
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
    
    fc = float(evolutionRule(int(gridValue), int(n)));
  } else {
    vec3 cursorVolumePos = computeCursorVolumePos();
    if (cursorVolumePos != vec3(0,0,0) && length(cursorVolumePos-posInVolume) < brushSize)
      fc = texture(noise, posInVolume).r < brushDensity ? 1.0 : 0.0;
    else
      fc = gridValue;
  }
  
}
