#include "Controller.h"
#include "CameraBehaviors.hpp"
#include "GLFW/glfw3.h"
#include "TextRendering.hpp"
#include "sgraph/IScenegraph.h"
#include "sgraph/Scenegraph.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/ScaleTransform.h"
#include "ObjImporter.h"
using namespace sgraph;
#include <iostream>
using namespace std;

#include "sgraph/ScenegraphExporter.h"
#include "sgraph/ScenegraphImporter.h"

#include <glm/ext/quaternion_trigonometric.hpp>

Controller::Controller(istream& scenegraphFile, Model& m,View& v): model(m), view(v) {
  initScenegraph(scenegraphFile);

  globalCamera = new StationaryCamera(glm::vec3(50.0, 75.0, 100.0));
  globalCamera->LookAt(glm::vec3(0.0,0.0,0.0));
  cameras.push_back(globalCamera);

  helicopterCamera = new HelicopterCamera(175, 150, glm::radians(30.0), 25);
  cameras.push_back(helicopterCamera);

  firstPersonCamera = new FirstPersonCamera(glm::vec3{0,30,75}, 10, glm::radians(60.0));
  cameras.push_back(firstPersonCamera);
}

void Controller::initScenegraph(istream& scenegraphFile) {
  sgraph::ScenegraphImporter importer;

  //read in the file of commands
  IScenegraph *scenegraph = importer.parse(scenegraphFile);
  //scenegraph->setMeshes(meshes);
  model.setScenegraph(scenegraph);
  cout <<"Scenegraph made" << endl;

  TextRendering tr;
  scenegraph->getRoot()->accept((sgraph::SGNodeVisitor *)&tr);
}

Controller::~Controller()
{
  if (globalCamera) delete globalCamera;
  if (helicopterCamera) delete helicopterCamera;
  if (firstPersonCamera) delete firstPersonCamera;
}

void Controller::run()
{
  sgraph::IScenegraph * scenegraph = model.getScenegraph();
  map<string,util::PolygonMesh<VertexAttrib> > meshes = scenegraph->getMeshes();

  view.init(this,model);

  float deltaTime = 0;
  while (!view.shouldWindowClose()) {
    for(auto& camera : cameras) {
      camera->Update(deltaTime);
    }

    deltaTime = view.display(scenegraph, cameras, cameras[activeCameraIndex]);
    if (deltaTime < 0) break;
  }
  view.closeWindow();
  exit(EXIT_SUCCESS);
}

void Controller::onkey(int key, int scancode, int action, int mods)
{
  // cout << (char)key << " pressed" << endl;

  switch (key) {
    // Camera Switching
    case GLFW_KEY_1:
      if (action == GLFW_PRESS) activeCameraIndex = 0;
      break;
    case GLFW_KEY_2:
      if (action == GLFW_PRESS) activeCameraIndex = 1;
      break;
    case GLFW_KEY_3:
      if (action == GLFW_PRESS) activeCameraIndex = 2;
      break;

    // FPS Camera Movement
    case GLFW_KEY_MINUS:
      if (action == GLFW_PRESS)
        firstPersonCamera->PressedButton(FirstPersonCamera::Input::MoveBackward);
      else if (action == GLFW_RELEASE)
        firstPersonCamera->ReleasedButton(FirstPersonCamera::Input::MoveBackward);
      break;
    case GLFW_KEY_EQUAL:
      if (action == GLFW_PRESS)
        firstPersonCamera->PressedButton(FirstPersonCamera::Input::MoveForward);
      else if (action == GLFW_RELEASE)
        firstPersonCamera->ReleasedButton(FirstPersonCamera::Input::MoveForward);
      break;
    case GLFW_KEY_UP:
      if (action == GLFW_PRESS)
        firstPersonCamera->PressedButton(FirstPersonCamera::Input::LookUp);
      else if (action == GLFW_RELEASE)
        firstPersonCamera->ReleasedButton(FirstPersonCamera::Input::LookUp);
      break;
    case GLFW_KEY_DOWN:
      if (action == GLFW_PRESS)
        firstPersonCamera->PressedButton(FirstPersonCamera::Input::LookDown);
      else if (action == GLFW_RELEASE)
        firstPersonCamera->ReleasedButton(FirstPersonCamera::Input::LookDown);
      break;
    case GLFW_KEY_LEFT:
      if (action == GLFW_PRESS)
        firstPersonCamera->PressedButton(FirstPersonCamera::Input::LookLeft);
      else if (action == GLFW_RELEASE)
        firstPersonCamera->ReleasedButton(FirstPersonCamera::Input::LookLeft);
      break;
    case GLFW_KEY_RIGHT:
      if (action == GLFW_PRESS)
        firstPersonCamera->PressedButton(FirstPersonCamera::Input::LookRight);
      else if (action == GLFW_RELEASE)
        firstPersonCamera->ReleasedButton(FirstPersonCamera::Input::LookRight);
      break;

    case GLFW_KEY_R:
      if (action == GLFW_PRESS) firstPersonCamera->Reset();
      break;
  }
}

void Controller::mousePosition(double xpos, double ypos) {
  // cout << xpos << ',' << ypos << endl;
  glm::vec2 currentDelta = glm::vec2(xpos, ypos) - currentMousePos;

  currentMousePos.x = xpos;
  currentMousePos.y = ypos;

  if (isMousePressed)
  {
    if (currentDelta == currentMousePos) return;
    glm::vec3 rotAxis = glm::vec3(currentDelta.y, currentDelta.x, 0);
    rotAxis = glm::normalize(rotAxis);
    // cout << "deltaMousePos: " << deltaMousePos.x << ", " << deltaMousePos.y << endl;
  }
}

void Controller::reshape(int width, int height) 
{
  cout <<"Window reshaped to width=" << width << " and height=" << height << endl;
  view.resize();
}

void Controller::dispose()
{
  view.closeWindow();
}

void Controller::error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

void Controller::mouseButton(int button, int action, int mods) {
  if (button != GLFW_MOUSE_BUTTON_LEFT) return;

  // cout << "button: " << button << " action: " << action << " mods: " << mods << endl;

  // Left click pressed.
  if (action == GLFW_PRESS)
    isMousePressed = true;
  // Left click released.
  else if (action == GLFW_RELEASE)
    isMousePressed = false;

  // cout << "isMousePressed: " << isMousePressed << endl;
}
