#version 410

uniform mat4 invV;
uniform vec3 vLightPos;

uniform float fractParam;
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

    vec2 z = start+delta*tc;
    vec2 c = vec2(fractParam*cos(animation),fractParam*sin(animation));
    int i = 1;
    
    while (i <= 255 && (z.x*z.x+z.y*z.y) < 4.0) {
        z = mulC(z,z)+c;
        i += 1;
    }
    i = i % 255;
    return float(i)/255;
}

vec3 colorMap(float f) {
    return fract(vec3(f*19,f*37,f*25));
}

void main() {
    vec3 texValue = colorMap(fractal(tc*vec2(1,2)-vec2(0,0.5)));
    
    vec3 lightDir = normalize(vLightPos-pos);
    vec3 nNormal = normalize(normal);

    vec3 eyePos = (invV * vec4(0.0,0.0,0.0,1.0)).xyz;
    vec3 eyeDir = normalize(eyePos-pos);

    vec3 r = reflect(-lightDir, nNormal);
    vec3 specular = vec3(1.0,1.0,1.0) * pow(max(0.0,dot(eyeDir,r)),22.0);
    vec3 diffuse  = texValue * max(0.0,dot(lightDir,nNormal));
    vec3 ambient  = texValue * vec3(0.2,0.2,0.2);
    
    FragColor = vec4(specular+diffuse+ambient, 1.0);
}
