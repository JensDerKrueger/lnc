#version 410

uniform sampler2D gridSampler;
uniform vec2 paintPos;

in vec2 tc;
out vec4 FragColor;

void main() {
  vec2 texSize = vec2(textureSize(gridSampler,0));
  if (paintPos.x < 0) {
    
    vec2 pixOffset[8] = vec2[]( vec2(-1,-1), vec2(0,-1), vec2(1,-1),
                                vec2(-1, 0),             vec2(1, 0),
                                vec2(-1, 1), vec2(0, 1), vec2(1, 1));
    
    float n = 0;
    for (int i = 0;i<8;i++) {
      n += texture(gridSampler, tc + pixOffset[i] / texSize).r;
    }

    float gridValue = texture(gridSampler, tc).r;

    float result;
    if (gridValue == 1)
      result = n == 2.0 || n == 3.0 ? 1.0 : 0.0;
    else
      result = n == 3.0 ? 1.0 : 0.0;
    
    FragColor = vec4(result, result, result, 1.0);
  } else {
    float gridValue = texture(gridSampler, tc).r;
    
    if (int(tc.x*texSize.x) == int(paintPos.x*texSize.y) &&
        int(tc.y*texSize.x) == int((1-paintPos.y)*texSize.y)) {
      gridValue = 1.0;
    }
    
    FragColor = vec4(gridValue, gridValue, gridValue, 1.0);
  }
}
