#version 410

in vec3 vNorm;
in vec3 vPos;
in vec3 vTan;
out vec3 normal;
out vec3 tang;

void main() {
	gl_Position = vec4(vPos, 1.0);
	normal = vNorm;
    tang = vTan;
}
