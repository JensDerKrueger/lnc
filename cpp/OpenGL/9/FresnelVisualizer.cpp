#include <string>

#include "FresnelVisualizer.h"

static std::string vsString{
"#version 410\n"
"uniform mat4 MV;\n"
"uniform mat4 MVit;\n"
"in vec3 vNorm;\n"
"in vec3 vPos;\n"
"in vec3 vTan;\n"
"out vec4 normal;\n"
"out vec4 tang;\n"
"out vec4 binorm;\n"
"void main() {\n"
"	gl_Position = MV * vec4(vPos, 1.0);\n"
"	normal = MVit * vec4(normalize(vNorm),0.0);\n"
"   tang   = MVit * vec4(normalize(vTan),0.0);\n"
"   binorm = MVit * vec4(cross(normalize(vTan),normalize(vNorm)),0.0);\n"
"}"};

static std::string gsString{
"#version 410 core\n"
"\n"
"layout(points) in;\n"
"layout(line_strip, max_vertices = 6) out;\n"
"\n"
"in vec4 normal[];\n"
"in vec4 tang[];\n"
"in vec4 binorm[];\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform mat4 P;\n"
"\n"
"const float LENGTH = 0.1;\n"
"\n"
"void drawLine(vec4 start, vec4 dir, vec4 c) {\n"
"    color = c;\n"
"    gl_Position = P * start;\n"
"    EmitVertex();\n"
"    gl_Position = P * (start + dir * LENGTH);\n"
"    EmitVertex();\n"
"    EndPrimitive();\n"
"}\n"
"\n"
"void main() {\n"
"    drawLine(gl_in[0].gl_Position, normal[0], vec4(1.0,0.0,0.0,1.0));\n"
"    drawLine(gl_in[0].gl_Position, tang[0],   vec4(0.0,1.0,0.0,1.0));\n"
"    drawLine(gl_in[0].gl_Position, binorm[0], vec4(0.0,0.0,1.0,1.0));\n"
"}\n"
};

static std::string fsString{
"#version 410\n"
"in vec4 color;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = color;\n"
"}\n"};


FresnelVisualizer::FresnelVisualizer(const GLBuffer& pos, const GLBuffer& norm, const GLBuffer& tang, uint32_t count) :
	visArray{},
	visProg(GLProgram::createFromString(vsString, fsString, gsString)),
	pLoc{visProg.getUniformLocation("P")},
	mvLoc{visProg.getUniformLocation("MV")},
	mvitLoc{visProg.getUniformLocation("MVit")},
	count(count)
{	
	visArray.bind();
	visArray.connectVertexAttrib(pos,  visProg, "vPos",3);
	visArray.connectVertexAttrib(norm, visProg, "vNorm",3);
	visArray.connectVertexAttrib(tang, visProg, "vTan",3);
}

void FresnelVisualizer::render(const Mat4& mv, const Mat4& p) const {
	glDisable(GL_DEPTH_TEST);

	// bind geometry
	visArray.bind();

	// setup transformations
	visProg.enable();      
				
	visProg.setUniform(pLoc, p);
	visProg.setUniform(mvLoc, mv);
	visProg.setUniform(mvitLoc, Mat4::inverse(mv), true);

	// render geometry
	glDrawArrays(GL_POINTS, 0, count);

	glEnable(GL_DEPTH_TEST);
}