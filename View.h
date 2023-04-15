#ifndef __VIEW_H__
#define __VIEW_H__

#include "../include/Light.h"
#include "sgraph/RaycastRenderer.hpp"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <ShaderProgram.h>
#include "sgraph/SGNodeVisitor.h"
#include "ObjectInstance.h"
#include "PolygonMesh.h"
#include "VertexAttrib.h"
#include "Camera.hpp"
#include "Callbacks.h"
#include "sgraph/IScenegraph.h"
#include "Model.h"

#include <stack>
using namespace std;


class View
{
  
    struct LightLocation {
        int ambient, diffuse, specular, position, spotDirection, spotCutoff;
        LightLocation() {
            ambient = diffuse = specular = position = spotDirection = spotCutoff = -1;
        }
    };

public:
    View(bool useOpenGl = false);
    ~View();
    void init(Callbacks* callbacks, Model& model);
    float display(sgraph::IScenegraph *scenegraph, vector<Camera*>& cameras, Camera* activeCamera);
    void resize();
    bool shouldWindowClose();
    void closeWindow();
    void output_raytrace(sgraph::IScenegraph *scenegraph, Camera* activeCamera);


private: 
    void initObjects(Model& model);
    vector<glm::vec4> getLightPositions(const glm::mat4& transformation);
    void initLights(Model& model);
    void initShaderVariables();

    GLFWwindow* window;
    float aspectRatio;
    util::ShaderProgram program;
    util::ShaderLocationsVault shaderLocations;
    map<string,util::ObjectInstance *> objects;
    map<string,util::TextureImage *> textures;
    util::ObjectInstance* cameraObj;
    glm::mat4 projection;
    stack<glm::mat4> modelview;
    sgraph::SGNodeVisitor *renderer;
    sgraph::RaycastRenderer *raycastRenderer;
    int frames;
    double time;
    double deltaTime;

    //the lights in our scene
    vector<util::Light> lights;
    //the shader locations for all lights, for convenience
    vector<LightLocation> lightLocations;
    //either name of object, or world, or view
    vector<string> lightCoordinateSystems;
    //texture ids
    map<string,unsigned int> textureIds;
    //mipmapping or not
    // bool mipmapped;

    bool useRaycast = false;

};

#endif
