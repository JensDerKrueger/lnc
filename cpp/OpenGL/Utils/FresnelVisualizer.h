#pragma once

#pragma once

#include <vector>

#include "Vec3.h"
#include "Mat4.h"
#include "GLProgram.h"
#include "GLBuffer.h"
#include "GLArray.h"


class FresnelVisualizer {
public:
	FresnelVisualizer(const GLBuffer& pos, const GLBuffer& norm, const GLBuffer& tang, uint32_t count, uint32_t start=0);
	void render(const Mat4& mv, const Mat4& p) const;
	
private:
	GLArray visArray;
	GLProgram visProg;
	GLint pLoc;
	GLint mvLoc;
	GLint mvitLoc;
	uint32_t count;
	uint32_t start;
};