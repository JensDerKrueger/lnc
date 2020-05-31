#version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec3 vPos;
varying vec3 color;

void main() {
	gl_Position = MVP * vec4(vPos, 1.0);
	color = vCol;
}
