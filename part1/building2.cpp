//! [code]

#include "Box.h"
#define GLM_FORCE_SWIZZLE
#include "BoxSet.h"
#include <fstream>
#include <math.h>
using namespace std;
#include "Material.h"

#define PI 3.14159265

int main(int argc,char *argv[]) {
    BoxSet bset;
    ofstream outStream("output_building2.txt");
    util::Material mat;
    mat.setAmbient(1,1,1);
    mat.setDiffuse(1,1,1);
    mat.setSpecular(1,1,1);
    mat.setShininess(10);
    //start with one box


    int steps = 10;
    int curStep = 0;
    int width = 10;

    bset.add(Box(0,0,0, width, 77,width));
    bset.add(Box(0,0,0, 90, 10,width)); // barrier
    bset.add(Box(0,0,10, width, 10,90)); // barrier
    bset.add(Box(90,0,10, width, 10,90)); // barrier
    bset.add(Box(90,0,0, width, 77,width));
    bset.add(Box(0,0,width * steps, width, 77,width));
    bset.add(Box(90,0,width * steps, width, 77,width));

    for (float i = 0; i < 2 * PI; i += (2 * PI) / steps)
    {
        cout << i << endl;
        bset.add(Box(0,sin(i) * width + 75,curStep * width,100,width/2,width));
        curStep++;
    }

    //write it out to the above file. Using the one-argument version of 
    //this function means it will export the scene graph with each box
    //of a different random color. When we incorporate lighting into the 
    //scene you should regenerate your model using a specific material,
    //by using the two-argument version of this function
    bset.toScenegraph(outStream);

}

//! [code]
