//
// Created by Roshan Patel on 4/4/23.
//

#ifndef CS4300_RAYTRACER_PPMIMAGEEXPORTER_H
#define CS4300_RAYTRACER_PPMIMAGEEXPORTER_H

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class PPMImageExporter {

public:
    PPMImageExporter() {

    }

    void export(std::string path, int width, int height, int factor, vector<glm::vec3> data) {
        std::string out = "";
        std::ofstream op;

        //read in the word P3
        out += "P3" + "\n";
        out += width + " " + height + "\n";
        out += "255\n";


//        for (i=0;i<height;i++)
//        {
//            for (j=0;j<width;j++)
//            {
//                input >> r >> g >> b;
//                image[3*((height-1-i)*width+j)] = r;
//                image[3*((height-1-i)*width+j)+1] = g;
//                image[3*((height-1-i)*width+j)+2] = b;
//            }
//        }
    }

};

#endif //CS4300_RAYTRACER_PPMIMAGEEXPORTER_H
