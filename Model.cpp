#include "Model.h"
#include "PolygonMesh.h"
#include "VertexAttrib.h"
#include <GLFW/glfw3.h>

Model::Model() {
  cameraMesh.setPrimitiveType(GL_LINES);
  cameraMesh.setPrimitiveSize(2);
  
  vector<VertexAttrib> cameraVertices;

  vector<float> vertexPositions = {
    0,0,0,
    0,0,2,
    -0.25,0.25,0.5,
    -0.5,0.5,1,
    0.25,0.25,0.5,
    0.5,0.5,1,
    0.25,-0.25,0.5,
    0.5,-0.5,1,
    -0.25,-0.25,0.5,
    -0.5,-0.5,1
  };

  for (size_t ii = 2; ii < vertexPositions.size(); ii += 3) {
    cameraVertices.emplace_back();
    cameraVertices.back().setData("position", {
      vertexPositions[ii-2],
      vertexPositions[ii-1],
      vertexPositions[ii]
    });
  }

  vector<unsigned int> cameraLines = {
    0, 1,

    2, 3,
    4, 5,
    6, 7,
    8, 9,

    2, 4, 4, 6, 6, 8, 8, 2,

    3, 5, 5, 7, 7, 9, 9, 3
  };

  cameraMesh.setVertexData(cameraVertices);
  cameraMesh.setPrimitives(cameraLines);
}

Model::~Model() {
    if (scenegraph) {
        delete scenegraph;
    }
}



sgraph::IScenegraph *Model::getScenegraph() {
    return this->scenegraph;
}

void Model::setScenegraph(sgraph::IScenegraph *scenegraph) {
    this->scenegraph = scenegraph;
}

util::PolygonMesh<VertexAttrib>& Model::getCameraMesh() {
  return cameraMesh;
}

