#version 410 core

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 Mit;
in vec3 vNorm;
in vec3 vPos;
in vec3 vTan;
in vec2 vTc;
out vec3 normal;
out vec3 tang;
out vec3 pos;
out vec2 tc;

void main() {
	gl_Position = MVP * vec4(vPos, 1.0);
    pos = (M * vec4(vPos, 1.0)).xyz;
	normal = (Mit * vec4(vNorm, 0.0)).xyz;
    tang = (Mit * vec4(vTan, 0.0)).xyz;
    tc = vTc;
}
