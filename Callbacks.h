#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

class Callbacks {
    public:
    virtual void reshape(int width, int height)=0;
    virtual void dispose()=0;
    virtual void onkey(int key, int scancode, int action, int mods)=0;
    virtual void mousePosition(double xpos, double ypos)=0;
    virtual void error_callback(int error, const char* description)=0;
    virtual void mouseButton(int button, int action, int mods)=0;
};

#endif
