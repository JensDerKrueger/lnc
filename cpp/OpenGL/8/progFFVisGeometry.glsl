#version 410 core

layout(points) in;
layout(line_strip, max_vertices = 6) out;

in vec3 normal[];
in vec3 tang[];

out vec4 color;

uniform mat4 MV;
uniform mat4 MVit;
uniform mat4 P;

const float LENGTH = 0.1;

void main() {
    
    
     //(Mit * vec4(vTan, 0.0)).xyz;
    vec4 viewPos = MV * gl_in[0].gl_Position;
    vec4 nNormal = MVit * vec4(normalize(normal[0]),0.0);
    vec4 nTang   = MVit * vec4(normalize(tang[0]),0.0);
    vec4 nBinorm = MVit * vec4(cross(normalize(tang[0]),normalize(normal[0])),0.0);

    color = vec4(1.0,0.0,0.0,1.0);
    gl_Position = P * viewPos;
    EmitVertex();
    gl_Position = P * (viewPos + nNormal * LENGTH);
    EmitVertex();
    EndPrimitive();

    color = vec4(0.0,1.0,0.0,1.0);
    gl_Position = P * viewPos;
    EmitVertex();
    gl_Position = P * (viewPos + nTang * LENGTH);
    EmitVertex();
    EndPrimitive();

    color = vec4(0.0,0.0,1.0,1.0);
    gl_Position = P * viewPos;
    EmitVertex();
    gl_Position = P * (viewPos + nBinorm * LENGTH);
    EmitVertex();
    EndPrimitive();
}
