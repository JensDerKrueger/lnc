#version 410

in vec3 tc;
out vec4 fc;

uniform sampler2D frontFaces;

void main() {
  vec3 entryPoint = texelFetch(frontFaces,ivec2(gl_FragCoord),0).xyz;
  vec3 exitPoint = tc;
  vec3 direction = normalize(exitPoint-entryPoint);
  
  fc = vec4(direction.x,direction.y,direction.z,1.0);
}
