#pragma once

#include <vector>

#include "Mat4.h"
#include "Vec3.h"
#include "Tesselation.h"
#include "GLBuffer.h"
#include "GLArray.h"

class PlanarMirror {
public:
	PlanarMirror(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d);
	
	void start(const Mat4& MVP) const;
	void end(const Mat4& MVP) const;
	Mat4 getMirrorMatrix() const {return mirrorMatrix;}

private:
	Mat4 mirrorMatrix;
	
	Tesselation mirrorTess;	
	GLBuffer vbMirror;
	GLBuffer ibMirror;
	GLArray arMirror;
	
	GLProgram prog;
	GLint mvpLoc;

	
};