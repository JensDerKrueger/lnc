#version 120

uniform sampler2D sprite;

varying vec4 color;

void main() {
    vec4 texValue = texture2D(sprite, gl_PointCoord).rgbr;
    gl_FragColor = vec4(color*texValue);
}
