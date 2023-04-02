#ifndef __MODEL_H__
#define __MODEL_H__

#include "PolygonMesh.h"
#include "VertexAttrib.h"
#include "sgraph/IScenegraph.h"
#include <map>
using namespace std;

class Model 
{
public:
    Model();
    ~Model();
    sgraph::IScenegraph *getScenegraph();
    void setScenegraph(sgraph::IScenegraph *scenegraph);
    util::PolygonMesh<VertexAttrib>& getCameraMesh();

private:
    
    sgraph::IScenegraph *scenegraph;
    util::PolygonMesh<VertexAttrib> cameraMesh;
};
#endif
