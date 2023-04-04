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
        string out = "";
        std::ofstream op;
        std::string str;
        int i,j;
        int r,g,b;
        int factor;

        fp.open(filename.c_str());

        if (!fp.is_open())
            throw std::invalid_argument("File not found!");

        std::cout << "Image file opened" << std::endl;

        //read line by line and ignore comments
        std::stringstream input;
        std::string line;

        while (std::getline(fp,line)) {
            if ((line.length()>0) && (line[0]!='#')) {
                input << line << std::endl;
            }
        }
        fp.close();


        //read in the word P3

        input >> str;


        //now read in the width and height

        input >> width >> height;

        //read in the factor
        input >> factor;

        image = new GLubyte[3*width*height];

        for (i=0;i<height;i++)
        {
            for (j=0;j<width;j++)
            {
                input >> r >> g >> b;
                image[3*((height-1-i)*width+j)] = r;
                image[3*((height-1-i)*width+j)+1] = g;
                image[3*((height-1-i)*width+j)+2] = b;
                // << "r=" << (int)r << ", g=" << (int)g << ",b=" << (int)b << endl;
            }
        }
        //fp.close();
    }

};

#endif //CS4300_RAYTRACER_PPMIMAGEEXPORTER_H
