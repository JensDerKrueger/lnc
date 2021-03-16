#version 410

uniform mat4 invV;

uniform vec3 vLightPos;

uniform sampler2D textureSampler;

in vec3 pos;
in vec3 normal;
in vec2 tc;

out vec4 fc;

void main() {
    vec3 gridValue = texture(textureSampler, tc).rgb;
    vec4 texValue;
    if (gridValue.g == 1) {
        texValue = vec4(1.0);
    } else if (gridValue.r > 0) {
        vec4 first = vec4(1.0, 1.0, 0.0, 1.0);
        vec4 last = vec4(0.3, 0.3, 0.7, 1.0);
        texValue = mix(first, last, gridValue.b);
    } else {
        texValue = vec4(0.0);
    }

    vec3 lightDir = normalize(vLightPos-pos);
    vec3 nNormal = normalize(normal);
    
    vec3 eyePos = (invV * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 eyeDir = normalize(eyePos-pos);

    vec3 r = reflect(-lightDir, nNormal);
    vec3 specular = vec3(0.7, 1.0, 0.8) * pow(max(0.0, dot(eyeDir, r)), 12.0);
    vec3 diffuse  = texValue.rgb * max(0.0, dot(lightDir, nNormal));
    vec3 ambient  = texValue.rgb * vec3(0.9, 0.9, 0.9);
    
    
    fc = vec4((specular+diffuse)*0.3+ambient, 1.0);
}
