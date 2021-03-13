#version 410

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 Mit;

in vec3 vPos;
in vec2 vTc;
in vec3 vNorm;

out vec3 pos;
out vec3 normal;
out vec2 tc;

void main() {
	gl_Position = MVP * vec4(vPos, 1.0);
    pos = (M * vec4(vPos, 1.0)).xyz;
    normal = (Mit * vec4(vNorm, 0.0)).xyz;
    tc = vTc;
}
