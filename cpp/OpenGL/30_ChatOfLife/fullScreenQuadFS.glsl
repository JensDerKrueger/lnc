#version 410

uniform sampler2D gridSampler;

in vec2 tc;
out vec4 FragColor;


//origin: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}



void main() {
  vec3 gridValue = texture(gridSampler, tc).rgb;
  
  if (gridValue.g == 1) {
    FragColor = vec4(1.0);
  } else if (gridValue.r > 0) {
    vec4 first = vec4(hsv2rgb(vec3(gridValue.r,1.0,1.0)),1.0);
    vec4 last = first*0.1;
    FragColor = mix(first,last,gridValue.b);
  } else {
    FragColor = vec4(0.0);
  }
}
