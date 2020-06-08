#version 410
uniform vec3 color;
uniform float opacity;

out vec4 FragColor;

in vec2 tc;

vec2 mulC(vec2 a, vec2 b) {
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y +a.y*b.x);
}

float fractal(vec2 tc) {
    float zoom = 1.0;
    float xs = -1.9;
    float ys = -1.3;

    float dx = (2.6/zoom);
    float dy = (2.6/zoom);

    vec2 c = vec2(xs+tc.x*dx,ys+tc.y*dy);
    vec2 z = vec2(0.0,0.0);
    int i = 1;
    
    while (i <= 255 && (z.x*z.x+z.y*z.y) < 8.0) {
        z = mulC(z,z)+c;
        i += 1;
    }
    
    if (i == 256) i = 0;
    
    return float(i)/25.0;
}


void main() {    
    float f = fractal(tc);
    FragColor = vec4(color * f, opacity);
}
