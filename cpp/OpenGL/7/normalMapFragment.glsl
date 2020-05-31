#version 120

uniform mat4 invV;

uniform vec3 vLightPos;
uniform float texRescale;

uniform sampler2D textureSampler;
uniform sampler2D normalSampler;

varying vec3 normal;
varying vec3 pos;
varying vec3 tang;
varying vec2 tc;

void main() {
    vec3 lightDir = normalize(vLightPos-pos);
    vec3 nNormal = normalize(normal);
    vec3 nTangent = normalize(tang);
    vec3 nBinorm = normalize(cross(nTangent,nNormal));
    
    nNormal = (normalize(mat3(nTangent,nBinorm,nNormal) * ((texture2D(normalSampler, tc*texRescale ).rgb-0.5)*2.0)));
    
    
    vec3 eyePos = (invV * vec4(0.0,0.0,0.0,1.0)).xyz;
    
    vec3 eyeDir = normalize(eyePos-pos);

    vec3 texValue = texture2D(textureSampler, tc*texRescale ).rgb;
    vec3 r = reflect(-lightDir, nNormal);
    
    vec3 specular = vec3(1.0,1.0,1.0) * pow(max(0.0,dot(eyeDir,r)),12.0);
    vec3 diffuse  = texValue * max(0.0,dot(lightDir,nNormal));
    vec3 ambient  = texValue * vec3(0.2,0.2,0.2);
    
    
    gl_FragColor = vec4(specular+diffuse+ambient, 1.0);
}
