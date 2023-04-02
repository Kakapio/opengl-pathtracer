//! [code]

#include "Box.h"
#define GLM_FORCE_SWIZZLE
#include "BoxSet.h"
#include <fstream>
using namespace std;
#include "Material.h"

int main(int argc,char *argv[]) {
    BoxSet bset;
    ofstream outStream("output_building1.txt");
    util::Material mat;
    mat.setAmbient(1,1,1);
    mat.setDiffuse(1,1,1);
    mat.setSpecular(1,1,1);
    mat.setShininess(10);

    int width = 32;
    int height = 120;
    //start with center box.
    bset.add(Box(0,0,0, width, height,width));
    bset.add(Box(6,height,6, width * 0.66,8,width * 0.66));

    // Our four towers. Total of 6 boxes so far.
    bset.add(Box(width,0,0,width,height,width));
    bset.add(Box(-width,0,0,width,height,width));
    bset.add(Box(0,0,width,width,height,width));
    bset.add(Box(0,0,-width,width,height,width));

    // Add windows to each tower. 12 boxes used here.
    for (int i = 0; i < 4; i++)
    {
        bset.difference(Box(width * 2 - 2, height - width * i, width / 2 - 2, 4, 10, 5));
        bset.difference(Box(-width - 2, height - width * i, width / 2 - 2, 4, 10, 5));
        bset.difference(Box(width / 2 - 2, height - width * i, width * 2 - 2, 5, 10, 4));
        bset.difference(Box(width / 2 - 2, height - width * i, -width - 2, 5, 10, 4));
    }

//    for(int i = 0; i < 3; i++)
//    {
//        bset.difference(Box(width*2, height - width * i, width/2, 1, 10, 5));
//        bset.difference(Box(width*-2, height - width * i, width/2, 1, 10, 5));
//    }

    //write it out to the above file. Using the one-argument version of 
    //this function means it will export the scene graph with each box
    //of a different random color. When we incorporate lighting into the 
    //scene you should regenerate your model using a specific material,
    //by using the two-argument version of this function
    bset.toScenegraph(outStream);

}

//! [code]
