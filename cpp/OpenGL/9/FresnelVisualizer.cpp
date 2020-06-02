#include <string>

#include "FresnelVisualizer.h"

static std::string vsString{
"#version 410\n"
"in vec3 vNorm;\n"
"in vec3 vPos;\n"
"in vec3 vTan;\n"
"out vec3 normal;\n"
"out vec3 tang;\n"
"void main() {\n"
"	gl_Position = vec4(vPos, 1.0);\n"
"	normal = vNorm;\n"
"    tang = vTan;\n"
"}"};

static std::string gsString{
"#version 410 core\n"
"\n"
"layout(points) in;\n"
"layout(line_strip, max_vertices = 6) out;\n"
"\n"
"in vec3 normal[];\n"
"in vec3 tang[];\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform mat4 MV;\n"
"uniform mat4 MVit;\n"
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
"    vec3 n =normalize(normal[0]);\n"
"    vec3 t =normalize(tang[0]);\n"
"\n"
"    vec4 viewPos = MV * gl_in[0].gl_Position;\n"
"    vec4 nNormal = MVit * vec4(n,0.0);\n"
"    vec4 nTang   = MVit * vec4(t,0.0);\n"
"    vec4 nBinorm = MVit * vec4(cross(t,n),0.0);\n"
"\n"
"    drawLine(viewPos, nNormal, vec4(1.0,0.0,0.0,1.0));\n"
"    drawLine(viewPos, nTang, vec4(0.0,1.0,0.0,1.0));\n"
"    drawLine(viewPos, nBinorm, vec4(0.0,0.0,1.0,1.0));\n"
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