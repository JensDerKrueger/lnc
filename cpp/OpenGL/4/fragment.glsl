#version 110

uniform vec3 vLightPos;

varying vec3 normal;
varying vec3 pos;
varying vec2 tc;

void main() {
    vec3 lightDir = normalize(vLightPos-pos);
    vec3 nnormal = normalize(normal);
    float diffuse = dot(lightDir,nnormal);
    float ambient = 0.3;
    gl_FragColor = vec4((ambient+diffuse)*tc.r, (ambient+diffuse)*tc.g, 0, 1.0);
}
