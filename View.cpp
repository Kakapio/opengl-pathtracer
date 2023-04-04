#include "View.h"
#include "GLFW/glfw3.h"
#include "../include/Light.h"
#include "PPMImageLoader.h"
#include "PolygonMesh.h"
#include "TextureImage.h"
#include "sgraph/LightAccumulator.h"
#include "sgraph/RaycastRenderer.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;
#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "sgraph/GLScenegraphRenderer.h"
#include "VertexAttrib.h"


View::View() {

}

View::~View(){

}

void View::init(Callbacks *callbacks, Model& model) 
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800, 800, "Hello GLFW: Per-vertex coloring", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
     glfwSetWindowUserPointer(window, (void *)callbacks);

    //using C++ functions as callbacks to a C-style library
    glfwSetKeyCallback(window, 
    [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->onkey(key,scancode,action,mods);
    });

    glfwSetWindowSizeCallback(window, 
    [](GLFWwindow* window, int width,int height)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->reshape(width,height);
    });

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, 
    [](GLFWwindow* window, double xpos, double ypos)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->mousePosition(xpos,ypos);
    });

    glfwSetMouseButtonCallback(window,
    [](GLFWwindow* window, int button, int action, int mods)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->mouseButton(button, action, mods);
    });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // create the shader program
    program.createProgram(string("shaders/phong-multiple.vert.glsl"),
                          string("shaders/phong-spot.frag.glsl"));
    // assuming it got created, get all the shader variables that it uses
    // so we can initialize them at some point
    // enable the shader program
    program.enable();
    shaderLocations = program.getAllShaderVariables();


    initObjects(model);
    initLights(model);
    initShaderVariables();
    resize();

    frames = 0;
    time = glfwGetTime();
    deltaTime = 0;

    renderer = new sgraph::GLScenegraphRenderer(modelview,objects,textureIds,shaderLocations);
    raycastRenderer = new sgraph::RaycastRenderer(modelview,objects);

    modelview.push(glm::mat4(1.0));
    model.getScenegraph()->getRoot()->accept(raycastRenderer);
    modelview.pop();
    
}

void View::initLights(Model& model) {
  modelview.push(glm::mat4(1.0));
  sgraph::LightAccumulator lightAccum(modelview);
  model.getScenegraph()->getRoot()->accept(&lightAccum);
  modelview.pop();

  for (auto light : lightAccum.getLights()) {
    lights.push_back(light);
    lightCoordinateSystems.push_back("world"); //in world
  }
}

void View::initObjects(Model& model) {
    map<string,string> shaderVarsToVertexAttribs;

    shaderVarsToVertexAttribs["vPosition"] = "position";
    shaderVarsToVertexAttribs["vNormal"] = "normal";
    shaderVarsToVertexAttribs["vTexCoord"] = "texcoord";

    map<string,util::PolygonMesh<VertexAttrib> > meshes = model.getScenegraph()->getMeshes();

    for (typename map<string,util::PolygonMesh<VertexAttrib> >::iterator it=meshes.begin();
           it!=meshes.end();
           it++) {
        std::cout << it->first << std::endl;
        util::ObjectInstance * obj = new util::ObjectInstance(it->first);
        obj->initPolygonMesh(shaderLocations,shaderVarsToVertexAttribs,it->second);
        objects[it->first] = obj;
    }

    //textures

    map<string,util::TextureImage *> textures = model.getScenegraph()->getTextures();
    
    PPMImageLoader ppmLoader;
    ImageLoader *imgLoader = &ppmLoader;
    imgLoader->load("textures/white.ppm");
    textures[""] = new util::TextureImage(imgLoader->getPixels(),imgLoader->getWidth(),imgLoader->getHeight(),"");

    glEnable(GL_TEXTURE_2D);
    for (auto& texturePair : textures) {
        util::TextureImage* textureObject = texturePair.second;
        unsigned int textureId;
        glGenTextures(1,&textureId);
        glBindTexture(GL_TEXTURE_2D,textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //if the s-coordinate goes outside (0,1), repeat it
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //if the t-coordinate goes outside (0,1), repeat it
	      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureObject->getWidth(),textureObject->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE,textureObject->getImage());
        glGenerateMipmap(GL_TEXTURE_2D);
        textureIds[texturePair.first] = textureId;
    }
    
    util::PolygonMesh<VertexAttrib> cameraMesh = model.getCameraMesh();
    cameraObj = new util::ObjectInstance("camera");
    cameraObj->initPolygonMesh(shaderLocations, shaderVarsToVertexAttribs, cameraMesh);
}

void View::initShaderVariables() {
    //get input variables that need to be given to the shader program
    for (int i = 0; i < lights.size(); i++)
    {
        LightLocation ll;
        stringstream name;

        name << "light[" << i << "]";
        ll.ambient = shaderLocations.getLocation(name.str() + "" +".ambient");
        ll.diffuse = shaderLocations.getLocation(name.str() + ".diffuse");
        ll.specular = shaderLocations.getLocation(name.str() + ".specular");
        ll.position = shaderLocations.getLocation(name.str() + ".position");
        ll.spotDirection = shaderLocations.getLocation(name.str() + ".spotDirection");
        ll.spotCutoff = shaderLocations.getLocation(name.str() + ".spotCutoff");
        lightLocations.push_back(ll);
    }
}



float View::display(sgraph::IScenegraph *scenegraph, vector<Camera*>& cameras, Camera* activeCamera) {

    program.enable();
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    modelview.push(glm::mat4(1.0));
    
    modelview.top() *= activeCamera->GetViewMatrix();
    glUniformMatrix4fv(shaderLocations.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //modelview currently represents world-to-view transformation
    //transform all lights into the view coordinate system before passing to
    //shaders. That way everything will be in one coordinate system in the shader
    //(the view) and the math will be correct

    for (int i = 0; i < lights.size(); i++) {
        glm::vec4 pos = lights[i].getPosition();
        glm::vec4 spotDir = lights[i].getSpotDirection();
        glm::mat4 lightTransformation;

        if (lightCoordinateSystems[i]=="view") {
            lightTransformation = glm::mat4(1.0);
        }
        else { //(lightCoordinateSystems[i]=="world") {
            lightTransformation = modelview.top();
        }
        /*
        else {
            lightTransformation = modelview.top() * model.getTransform(lightCoordinateSystems[i]);
        }
        */
        pos = lightTransformation * pos;
        glUniform4fv(lightLocations[i].position, 1, glm::value_ptr(pos));
        spotDir = lightTransformation * spotDir;
        glUniform3fv(lightLocations[i].spotDirection, 1, glm::value_ptr(glm::normalize(spotDir)));
        glUniform1f(lightLocations[i].spotCutoff, cosf(lights[i].getSpotCutoff()));
    }

    //pass light color properties to shader
    glUniform1i(shaderLocations.getLocation("numLights"),lights.size());
    
    //pass light colors to the shader
    for (int i = 0; i < lights.size(); i++) {
        glUniform3fv(lightLocations[i].ambient, 1, glm::value_ptr(lights[i].getAmbient()));
        glUniform3fv(lightLocations[i].diffuse, 1, glm::value_ptr(lights[i].getDiffuse()));
        glUniform3fv(lightLocations[i].specular, 1,glm::value_ptr(lights[i].getSpecular()));
    }

    glUniform1i(shaderLocations.getLocation("useOldShader"), false);
    //draw scene graph here
    scenegraph->getRoot()->accept(renderer);

    for (auto& camera : cameras) {
      if (camera == activeCamera) continue;


      glUniform1i(shaderLocations.getLocation("useOldShader"), true);
      glUniform4f(shaderLocations.getLocation("vColor"), 0.0, 1.0, 0.0, 1.0);
      glUniformMatrix4fv(shaderLocations.getLocation("modelview"), 1, GL_FALSE, glm::value_ptr(camera->GetModelviewMatrix(modelview.top(), aspectRatio, glm::radians(60.0f))));
      cameraObj->draw();
    }
    
    modelview.pop();
    glFlush();
    program.disable();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
    frames++;
    double currenttime = glfwGetTime();
    deltaTime = currenttime - time;
    time = currenttime;

    return deltaTime;
}

void View::resize() {
	  int window_width,window_height;
    glfwGetFramebufferSize(window,&window_width,&window_height);

    aspectRatio = (float)window_width/window_height;

    //prepare the projection matrix for perspective projection
	  projection = glm::perspective(glm::radians(60.0f),aspectRatio,0.1f,10000.0f);
    glViewport(0, 0, window_width,window_height);
}

bool View::shouldWindowClose() {
    return glfwWindowShouldClose(window);
}



void View::closeWindow() {
    for (map<string,util::ObjectInstance *>::iterator it=objects.begin();
           it!=objects.end();
           it++) {
          it->second->cleanup();
          delete it->second;
    }

    cameraObj->cleanup();
    delete cameraObj;

    glfwDestroyWindow(window);

    glfwTerminate();
}





