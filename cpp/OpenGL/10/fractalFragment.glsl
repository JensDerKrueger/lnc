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

vec3 astroIntensity(float t) {
    t = pow(t, 0.4);
    float pi = 3.1415926;
    float s = 2.1;
    float r = 1.53;
    float h = 2.0;
    float g = 1.0;
    float p1 = pow(t, g);
    float psi = 2.0*pi * (s/3.0 + r*t);
    float a = h*p1*(1.0-p1)/2.0;
    float cospsi = cos(psi);
    float sinpsi = sin(psi);
    vec3 result = vec3(-0.14861*cospsi + 1.78277*sinpsi, -0.29227*cospsi -0.90649*sinpsi, 1.97294*cospsi);
    return p1+a*result;
}

void main() {
    vec3 texValue = astroIntensity(fractal(tc*vec2(1, 2)-vec2(0, 0.5)));
    
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
