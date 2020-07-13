#version 410
uniform vec3 lightPos;
uniform sampler2D heightSampler;
in vec3 normal;
in vec3 pos;
in float gradient;
in float height;
uniform float reduction;
uniform float alpha;
out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(lightPos-pos);
    float diffuse = dot(normalize(normal), lightDir);
    //vec4 texValue = texture(heightSampler, height*reduction);
    
    vec4 texValue = texture(heightSampler, vec2(gradient,height*reduction));
    
    vec3 eyeDir = normalize(-pos);
    vec3 r = reflect(-lightDir, normal);
    
    vec3 specularColor = vec3(1.0,1.0,1.0) * pow(max(0.0,dot(eyeDir,r)),12.0);
    vec3 diffuseColor = texValue.xyz*diffuse;
    
    FragColor = vec4(diffuseColor+specularColor*texValue.a,alpha);
}
