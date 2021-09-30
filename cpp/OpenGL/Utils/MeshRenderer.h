#pragma once

#include <string>
#include <memory>

#include <GLProgram.h>
#include <GLArray.h>
#include <Tesselation.h>

class MeshRenderer {
public:
  MeshRenderer() {}

  MeshRenderer(const Tesselation& tesselation,
               bool useNormals=true,
               bool useTexCoords=false,
               bool useTangents=false) {
    init(tesselation, useNormals, useTexCoords, useTangents);
  }

  void init(const Tesselation& tesselation,
            bool useNormals=false,
            bool useTexCoords=false,
            bool useTangents=false) {

    glPosBuffer = std::make_shared<GLBuffer>(GL_ARRAY_BUFFER);

    glArray.bind();
    glPosBuffer->setData(tesselation.getVertices(), 3);

    if (useNormals) {
      glNormalBuffer = std::make_shared<GLBuffer>(GL_ARRAY_BUFFER);
      glNormalBuffer->setData(tesselation.getNormals(), 3);
    }

    if (useTexCoords) {
      glTexCoordsBuffer = std::make_shared<GLBuffer>(GL_ARRAY_BUFFER);
      glTexCoordsBuffer->setData(tesselation.getTexCoords(), 2);
    }

    if (useTangents) {
      glTangentBuffer = std::make_shared<GLBuffer>(GL_ARRAY_BUFFER);
      glTangentBuffer->setData(tesselation.getTangents(), 3);
    }

    glIndexBuffer.setData(tesselation.getIndices());
    vertexCount = GLsizei(tesselation.getIndices().size());
  }

  void connect(const GLProgram& program,
               const std::string& posVar = "vPos",
               const std::string& normalVar= "vNormal",
               const std::string& texCoordsVar= "vTC",
               const std::string& tagentVar= "vTang") {

    glArray.bind();
    glArray.connectVertexAttrib(*glPosBuffer, program, posVar, 3);
    if (glNormalBuffer)
      glArray.connectVertexAttrib(*glNormalBuffer, program, normalVar, 3);
    if (glTexCoordsBuffer)
      glArray.connectVertexAttrib(*glTexCoordsBuffer, program, texCoordsVar, 2);
    if (glTangentBuffer)
      glArray.connectVertexAttrib(*glTangentBuffer, program, tagentVar, 3);
    glArray.connectIndexBuffer(glIndexBuffer);
  }

  void render() {
    glArray.bind();
    GL(glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void*)0));
  }

private:
  GLArray     glArray;
  std::shared_ptr<GLBuffer>  glPosBuffer{nullptr};
  std::shared_ptr<GLBuffer>  glNormalBuffer{nullptr};
  std::shared_ptr<GLBuffer>  glTexCoordsBuffer{nullptr};
  std::shared_ptr<GLBuffer>  glTangentBuffer{nullptr};
  GLBuffer  glIndexBuffer{GL_ELEMENT_ARRAY_BUFFER};
  GLsizei     vertexCount{0};

};
