#version 410

uniform sampler2D gridSampler;

in vec2 tc;
out vec4 FragColor;

void main() {
  vec3 gridValue = texture(gridSampler, tc).rgb;
  
  if (gridValue.g == 1) {
    FragColor = vec4(1.0);
  } else if (gridValue.r > 0) {
    vec4 first = vec4(1.0,1.0,0.0,1.0);
    vec4 last = vec4(0.0,0.0,0.5,1.0);
    FragColor = mix(first,last,gridValue.b);
  } else {
    FragColor = vec4(0.0);
  }
}
