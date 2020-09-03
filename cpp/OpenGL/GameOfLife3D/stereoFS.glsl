#version 410

uniform sampler2D leftEye;
uniform sampler2D rightEye;

out vec4 fc;

void main() {
  vec4 leftPixel = texelFetch(leftEye, ivec2(gl_FragCoord.xy), 0);
  vec4 rightPixel = texelFetch(rightEye, ivec2(gl_FragCoord.xy), 0);
  
  float fGrayLeftEye  = dot(leftPixel.rgb, vec3(0.3,0.59,0.11));
  float fGrayRightEye = dot(rightPixel.rgb, vec3(0.3,0.59,0.11));

  fc = vec4(fGrayLeftEye, 0.0, fGrayRightEye, 1.0);
}
