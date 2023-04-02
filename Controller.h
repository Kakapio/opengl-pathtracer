#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "View.h"
#include "Model.h"
#include "Callbacks.h"
#include "CameraBehaviors.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Controller: public Callbacks
{
public:
    Controller(istream& scenegraphFile, Model& m,View& v);
    ~Controller();
    void run();

    virtual void reshape(int width, int height);
    virtual void dispose();
    virtual void onkey(int key, int scancode, int action, int mods);
    virtual void mousePosition(double xpos, double ypos);
    virtual void error_callback(int error, const char* description);
    virtual void mouseButton(int button, int action, int mods);
private:
    void initScenegraph(istream& scenegraphFile);

    View& view;
    Model& model;

    StationaryCamera* globalCamera, *globalCamera2;
    HelicopterCamera* helicopterCamera;
    FirstPersonCamera* firstPersonCamera;

    vector<Camera*> cameras;
    int activeCameraIndex = 0;

    glm::vec2 currentMousePos;
    bool isMousePressed;

};

#endif
