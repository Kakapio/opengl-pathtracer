#ifndef _RAYCASTRENDERER_H_
#define _RAYCASTRENDERER_H_

#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include <OpenGL/OpenGL.h>
#include <ShaderProgram.h>
#include <ShaderLocationsVault.h>
#include "ObjectInstance.h"
#include <algorithm>
#include <cmath>
#include <stack>
#include <iostream>
#include <unordered_map>
#include "../Ray3D.hpp"
#include <vector>
#include "../HitRecord.hpp"
#include "../PPMImageExporter.h"

using namespace std;

namespace sgraph {
    /**
     * This visitor implements drawing the scene graph using OpenGL
     * 
     */
    class RaycastRenderer: public SGNodeVisitor {
        public:
        /**
         * @brief Construct a new RaycastRenderer object
         * 
         * @param mv a reference to modelview stack that will be used while rendering
         * @param os the map of ObjectInstance objects
         * @param shaderLocations the shader locations for the program used to render
         */
        RaycastRenderer(stack<glm::mat4>& mv, map<string,util::ObjectInstance *>& os, string outfileLoc) 
            : modelview(mv),
              objects(os),
              outfileLoc(outfileLoc) {
            for (map<string,util::ObjectInstance *>::iterator it=objects.begin();it!=objects.end();it++) {
                cout << "Mesh with name: "<< it->first << endl;
            }
        }

        /**
         * @brief Recur to the children for drawing
         * 
         * @param groupNode 
         */
        void visitGroupNode(GroupNode *groupNode) {
            for (int i=0;i<groupNode->getChildren().size();i=i+1) {
                groupNode->getChildren()[i]->accept(this);
            }
        }

        /**
         * @brief Draw the instance for the leaf, after passing the 
         * modelview and color to the shader
         * 
         * @param leafNode 
         */
        void visitLeafNode(LeafNode *leafNode) {
            
            util::Material mat = leafNode->getMaterial();
            
            //send modelview matrix to GPU  
            glm::mat4 normalmatrix = glm::inverse(glm::transpose((modelview.top())));

            string modelType = leafNode->getInstanceOf();
            if (modelType != "box" && modelType != "sphere") return;

            string name = leafNode->getName();
            objTypeMap.emplace(name, modelType);
            modelviewMap.emplace(name, modelview.top());
            normalmatrixMap.emplace(name, normalmatrix);
            
        }

        /**
         * @brief Multiply the transform to the modelview and recur to child
         * 
         * @param transformNode 
         */
        void visitTransformNode(TransformNode * transformNode) {
            modelview.push(modelview.top());
            modelview.top() = modelview.top() * transformNode->getTransform();
            if (transformNode->getChildren().size()>0) {
                transformNode->getChildren()[0]->accept(this);
            }
            modelview.pop();
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         * 
         * @param scaleNode 
         */
        void visitScaleTransform(ScaleTransform *scaleNode) {
            visitTransformNode(scaleNode);
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         * 
         * @param translateNode 
         */
        void visitTranslateTransform(TranslateTransform *translateNode) {
            visitTransformNode(translateNode);
        }

        void visitRotateTransform(RotateTransform *rotateNode) {
            visitTransformNode(rotateNode);
        }

        //TODO: Add screen dimension parameters to renderer constructor
        Ray3D screenSpaceToViewSpace(float width, float height, glm::vec2 pos, float angle) {
            float halfWidth = width / 2.0f;
            float halfHeight = height / 2.0f;
            float aspect = width / height;

            Ray3D out = Ray3D(glm::vec3(0,0,0), glm::vec3(pos.x - halfWidth, pos.y - halfHeight,
                                                          -1 * (halfHeight / tan(angle))));
            return out;
        }

        void raycastBox(Ray3D& ray, HitRecord& hit, string& name) {

        }

        void raycastSphere(Ray3D& ray, Ray3D& objSpaceRay, HitRecord& hit, string& name) {
          // Solve quadratic
          float A = objSpaceRay.direction.x * objSpaceRay.direction.x + objSpaceRay.direction.y * objSpaceRay.direction.y + objSpaceRay.direction.z * objSpaceRay.direction.z;
          float B = 2.f * (objSpaceRay.direction.x * objSpaceRay.start.x + objSpaceRay.direction.y * objSpaceRay.start.y + objSpaceRay.direction.z * objSpaceRay.start.z);
          float C = objSpaceRay.start.x * objSpaceRay.start.x + objSpaceRay.start.y * objSpaceRay.start.y + objSpaceRay.start.z * objSpaceRay.start.z - 1.f;

          float root = sqrtf(B * B - 4.f * A * C);

          // no intersection
          if (root < 0) return;

          float t1 = (-B + root) / (2.f * A);
          float t2 = (B + root) / (2.f * A);

          float tMin = (t1 >= 0 && t2 >= 0) ? min(t1, t2) : max(t1, t2);
          // object is fully behind camera
          if (tMin < 0) return;

          // already hit a closer object
          if (hit.time <= tMin) return;

          hit.time = tMin;
          glm::vec4 objSpaceIntersection = objSpaceRay.start + tMin * objSpaceRay.direction;
          objSpaceIntersection.w = 0.f;
          hit.intersection = modelviewMap[name] * objSpaceIntersection; //ray.start + tMin * ray.direction;
          hit.normal = glm::normalize(normalmatrixMap[name] * objSpaceIntersection);
          // TODO: hit.mat = obj mat
        }

        void raycast(Ray3D& ray, HitRecord& hit) {

            for(auto& obj : objTypeMap)
            {
                string name = obj.first;
                glm::mat4 mv = modelviewMap[name];
                glm::mat4 mvInverse = glm::inverse(mv);

                // WARN: Might have to normalize direction vec
                Ray3D objSpaceRay(mvInverse * ray.start, mvInverse * ray.direction);

                if (obj.second == "box") raycastBox(objSpaceRay, hit, name);
                else if (obj.second == "sphere") raycastSphere(ray, objSpaceRay, hit, name);
            }
        }

        void raytrace(int width, int height, stack<glm::mat4>& mv) {
            rayHits.resize(height);
            pixelData.resize(height);

            for (int jj = 0; jj < height; ++jj) {
                vector<HitRecord>& hitsRow = rayHits[jj];
                hitsRow.resize(width);
                pixelData[jj].resize(width);

                for (int ii = 0; ii < width; ++ii) {
                    Ray3D ray = screenSpaceToViewSpace(width, height, glm::vec2(ii,height-jj), glm::radians(60.0));

                    raycast(ray, hitsRow[ii]);
                }
            }

            for (int jj = 0; jj < height; ++jj) {
                for (int ii = 0; ii < width; ++ii) {
                    if (rayHits[jj][ii].time < MaxFloat) {
                        pixelData[jj][ii] = glm::vec3(255, 255, 255);
                    }
                }
            }

            // TODO: export to file 'outfileLoc'
            PPMImageExporter ppmExporter;
            ppmExporter.export(outfileLoc, width, height, pixelData);
        }

        private:
        stack<glm::mat4>& modelview;    
        map<string,util::ObjectInstance *> objects;
        // TODO: map<string,util::TextureImage*> textures;
        unordered_map<string,glm::mat4> modelviewMap;
        unordered_map<string,glm::mat4> normalmatrixMap;
        unordered_map<string,string> objTypeMap;

        string outfileLoc;
        vector<vector<HitRecord> > rayHits;
        vector<vector<glm::vec3> > pixelData;
   };
}

#endif
