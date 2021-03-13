#version 410

uniform mat4 invV;
uniform vec3 vLightPos;

uniform vec3 color;
uniform float opacity;
uniform float animation;

out vec4 FragColor;

in vec3 normal;
in vec3 pos;
in vec2 tc;

vec2 mulC(vec2 a, vec2 b) {
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y +a.y*b.x);
}

float fractal(vec2 tc) {
    vec2 start = vec2(-3.0, -2.0);
    vec2 end   = vec2( 3.0,  2.0);
    vec2 delta = end-start;

    float r = 0.9;
    vec2 z = start+delta*tc;
    vec2 c = vec2(r*cos(animation),r*sin(animation));
    int i = 1;
    
    while (i <= 255 && (z.x*z.x+z.y*z.y) < 4.0) {
        z = mulC(z,z)+c;
        i += 1;
    }
    
    return float(i)/255;
}


void main() {
    vec3 texValue = color * clamp(pow(fractal(tc)*2,0.2),0,1);
    
    vec3 lightDir = normalize(vLightPos-pos);
    vec3 nNormal = normalize(normal);

    vec3 eyePos = (invV * vec4(0.0,0.0,0.0,1.0)).xyz;
    vec3 eyeDir = normalize(eyePos-pos);

    vec3 r = reflect(-lightDir, nNormal);
    vec3 specular = vec3(1.0,1.0,1.0) * pow(max(0.0,dot(eyeDir,r)),12.0);
    vec3 diffuse  = texValue * max(0.0,dot(lightDir,nNormal));
    vec3 ambient  = texValue * vec3(0.2,0.2,0.2);
    
    FragColor = vec4(specular+diffuse+ambient, opacity);
}
