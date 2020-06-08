#version 410

uniform mat4 MVP;

in vec3 vPos;
in vec2 vTc;

out vec2 tc;

void main() {
	gl_Position = MVP * vec4(vPos, 1.0);
    tc = vTc;
}
