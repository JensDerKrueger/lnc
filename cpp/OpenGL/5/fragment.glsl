#version 110

uniform mat4 invV;

uniform vec3 vLightPos;

uniform sampler2D textureSampler;
uniform sampler2D textureSampler2;

varying vec3 normal;
varying vec3 pos;
varying vec2 tc;

void main() {
    vec3 lightDir = normalize(vLightPos-pos);
    vec3 nNormal = normalize(normal);
    
    vec3 eyePos = (invV * vec4(0.0,0.0,0.0,1.0)).xyz;
    
    vec3 eyeDir = normalize(eyePos-pos);

    vec3 texValue = texture2D(textureSampler, tc*10.0 ).rgb + texture2D(textureSampler2, tc*10.0 ).rgb;
    vec3 r = reflect(-lightDir, nNormal);
    
    vec3 specular = vec3(1.0,1.0,1.0) * pow(max(0.0,dot(eyeDir,r)),12.0);
    vec3 diffuse  = texValue * max(0.0,dot(lightDir,nNormal));
    vec3 ambient  = texValue * vec3(0.2,0.2,0.2);
    
    
    gl_FragColor = vec4(specular+diffuse+ambient, 1.0);
}
