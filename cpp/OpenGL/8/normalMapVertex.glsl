#version 120

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 Mit;
attribute vec3 vNorm;
attribute vec3 vPos;
attribute vec3 vTan;
attribute vec2 vTc;
varying vec3 normal;
varying vec3 tang;
varying vec3 pos;
varying vec2 tc;

void main() {
	gl_Position = MVP * vec4(vPos, 1.0);
    pos = (M * vec4(vPos, 1.0)).xyz;
	normal = (Mit * vec4(vNorm, 0.0)).xyz;
    tang = (Mit * vec4(vTan, 0.0)).xyz;
    tc = vTc;
}
