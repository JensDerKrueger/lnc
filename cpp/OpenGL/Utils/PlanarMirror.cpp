#include "PlanarMirror.h"

static std::string vsString{
"#version 410\n"
"uniform mat4 MVP;\n"
"in vec3 vPos;\n"
"void main() {\n"
"	gl_Position = MVP * vec4(vPos, 1.0);\n"
"}"};

static std::string fsString{
"#version 410\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = vec4(1.0,1.0,0.0,0.5);\n"
"}\n"};

PlanarMirror::PlanarMirror(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) :
	mirrorTess{Tesselation::genRectangle(a,b,c,d)},
	vbMirror{GL_ARRAY_BUFFER},
	ibMirror{GL_ELEMENT_ARRAY_BUFFER},
	arMirror{},
	prog(GLProgram::createFromString(vsString, fsString)),
	mvpLoc{prog.getUniformLocation("MVP")}
{	
	vbMirror.setData(mirrorTess.getVertices(),3);
	ibMirror.setData(mirrorTess.getIndices());
	
	arMirror.bind();
	arMirror.connectVertexAttrib(vbMirror, prog, "vPos", 3);
	arMirror.connectIndexBuffer(ibMirror);
	
	const Vec3& p = a;
	const Vec3 d1 = b - p;
	const Vec3 d2 = c - p;
	const Vec3 n  = Vec3::normalize(Vec3::cross(d2,d1));
	
	mirrorMatrix = Mat4::mirror(p, n);
}



void PlanarMirror::start(const Mat4& MVP) const {
	GL(glEnable(GL_STENCIL_TEST));
	GL(glClearStencil(0));
	GL(glClear(GL_STENCIL_BUFFER_BIT));
	GL(glStencilFunc(GL_NEVER, 1, 0xffffffff));
	GL(glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE));
	
	
	GL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
	
	prog.enable();
	prog.setUniform(mvpLoc, MVP);
	arMirror.bind();
	GL(glDrawElements(GL_TRIANGLES, GLsizei(mirrorTess.getIndices().size()), GL_UNSIGNED_INT, (void*)0));
	
	GL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	
	GL(glStencilFunc(GL_EQUAL, 1, 0xffffffff));
	GL(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
	GL(glCullFace(GL_FRONT));
}


void PlanarMirror::end(const Mat4& MVP) const {
	GL(glDisable(GL_STENCIL_TEST));
	GL(glCullFace(GL_BACK));

	GL(glEnable(GL_BLEND));
	GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL(glBlendEquation(GL_FUNC_ADD));

	prog.enable();
	prog.setUniform(mvpLoc, MVP);
	arMirror.bind();
	GL(glDrawElements(GL_TRIANGLES, GLsizei(mirrorTess.getIndices().size()), GL_UNSIGNED_INT, (void*)0));
	
	GL(glDisable(GL_BLEND));
}
