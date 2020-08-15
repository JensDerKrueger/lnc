#version 410

uniform sampler2D gridSampler;

in vec2 tc;
out vec4 FragColor;

void main() {
  float gridValue = texture(gridSampler, tc).r;
  
  float left = texture(gridSampler, tc + vec2(-1,0)).r;
  float right = texture(gridSampler, tc + vec2(1,0)).r;
  float top = texture(gridSampler, tc + vec2(0,1)).r;
  float bottom = texture(gridSampler, tc + vec2(0,-1)).r;
  
  float n = left+right .....
  
  float result;
  if (gridValue == 1)
    result =  round(n) == 2.0 || n == 3.0 ? 1.0 : 0.0;
  else
    result = n == 3.0 ? 1.0 : 0.0;
  
  FragColor = vec4(result, result, result, 1.0);
}
