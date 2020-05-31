#version 120

uniform mat4 MVP;
attribute vec3 vPos;
attribute vec4 vColor;

varying vec4 color;

void main() {
	gl_Position = MVP * vec4(vPos, 1.0);
    color = vColor;
}
