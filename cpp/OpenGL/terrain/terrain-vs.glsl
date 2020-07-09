#version 410
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 itMV;
in vec3 vPos;
in vec3 vNormal;
in vec4 vColor;
out vec4 color;
out vec3 normal;
out vec3 pos;
out float height;

void main() {
    gl_Position = MVP * vec4(vPos/vec3(1.0,1.0,1.0), 1.0);
    normal = (itMV * vec4(vNormal, 0.0)).xyz;
    height = vPos.y;
    pos = (MV * vec4(vPos, 1.0)).xyz;
    color = vColor;
}
