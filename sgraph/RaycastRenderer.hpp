#ifndef _RAYCASTRENDERER_H_
#define _RAYCASTRENDERER_H_

#include "Light.h"
#include "Material.h"
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
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/geometric.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_inverse.hpp>

using namespace std;

namespace sgraph {
    /**
     * This visitor implements drawing the scene graph using OpenGL
     * 
     */
    class RaycastRenderer: public SGNodeVisitor {
        struct RaycastObj {
            string objType;

            glm::mat4 modelview;
            glm::mat4 modelviewInverse;
            glm::mat4 normalMat;
            
            util::Material mat;

            RaycastObj(string& type, glm::mat4 mv, util::Material& mat):
              objType(type),
              modelview(mv),
              modelviewInverse(glm::inverse(mv)),
              normalMat(glm::inverseTranspose(mv)),
              mat(mat) {
              cout << objType << endl;
              cout << glm::to_string(modelview) << endl;
              cout << glm::to_string(modelviewInverse) << endl;
              cout << glm::to_string(normalMat) << endl;
              cout << endl;
            }

        };

        public:
        /**
         * @brief Construct a new RaycastRenderer object
         * 
         * @param mv a reference to modelview stack that will be used while rendering
         * @param os the map of ObjectInstance objects
         * @param shaderLocations the shader locations for the program used to render
         */
        RaycastRenderer(stack<glm::mat4>& mv, vector<util::Light>& lights, string outfileLoc) 
            : modelview(mv),
              lights(lights),
              outfileLoc(outfileLoc) {
          for (auto& light : this->lights) {
            glm::vec4 lightPosition = light.getPosition();
            // lightPosition.z = -lightPosition.z;
            light.setPosition(modelview.top() * lightPosition);
            glm::vec4 spotDir = glm::normalize(modelview.top() * light.getSpotDirection());
            light.setSpotDirection(spotDir.x, spotDir.y, spotDir.z);
            light.setSpotAngle(cosf(light.getSpotCutoff()));
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
            string modelType = leafNode->getInstanceOf();
            if (modelType != "box" && modelType != "sphere") return;

            glm::mat4 mv = modelview.top();
            util::Material mat = leafNode->getMaterial();

            objs.emplace_back(modelType, mv, mat);
        }

        /**
         * @brief Multiply the transform to the modelview and recur to child
         * 
         * @param transformNode 
         */
        void visitTransformNode(TransformNode * transformNode) {
            modelview.push(modelview.top());
            glm::mat4 mv = modelview.top();
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
                                                          -(halfHeight / tan(angle))));
            return out;
        }


        bool intersectsWidthBoxSide(float& tMin, float& tMax, float start, float dir) {
          float t1 = (-0.5f - start);
          float t2 = (0.5f - start);
          if (dir == 0) {
            // no intersection
            if (glm::sign(t1) == glm::sign(t2)) return false;

            tMin = -MaxFloat;
            tMax = MaxFloat;
            return true;
          }

          t1 /= dir;
          t2 /= dir;

          if (dir < 0) {
            tMin = min(t1, t2);
            tMax = max(t1, t2);
          } else {
            tMin = t1;
            tMax = t2;
          }

          return true;
        }

        void raycastBox(Ray3D& ray, Ray3D& objSpaceRay, HitRecord& hit, RaycastObj& obj) {
          float txMin, txMax, tyMin, tyMax, tzMin, tzMax;

          if (!intersectsWidthBoxSide(txMin, txMax, objSpaceRay.start.x, objSpaceRay.direction.x))
            return;

          if (!intersectsWidthBoxSide(tyMin, tyMax, objSpaceRay.start.y, objSpaceRay.direction.y))
            return;

          if (!intersectsWidthBoxSide(tzMin, tzMax, objSpaceRay.start.z, objSpaceRay.direction.z))
            return;

          float tMin = max(max(txMin, tyMin), tzMin);
          float tMax = min(min(txMax, tyMax), tzMax);

          // no intersection
          if (tMax < tMin) return;

          float tHit = (tMin >= 0 && tMax >= 0) ? min(tMin, tMax) : max(tMin, tMax);
          // object is fully behind camera
          if (tHit < 0) return;

          // already hit a closer object
          if (hit.time <= tHit) return;

          glm::vec4 objSpaceIntersection = objSpaceRay.start + tHit * objSpaceRay.direction;
          objSpaceIntersection.w = 0.f;

          glm::vec4 objSpaceNormal(0.f,0.f,0.f,0.f);
          if (objSpaceIntersection.x > 0.4998f) objSpaceNormal.x += 1.f;
          else if (objSpaceIntersection.x < -0.4998f) objSpaceNormal.x -= 1.f;

          if (objSpaceIntersection.y > 0.4998f) objSpaceNormal.y += 1.f;
          else if (objSpaceIntersection.y < -0.4998f) objSpaceNormal.y -= 1.f;

          if (objSpaceIntersection.z > 0.4998f) objSpaceNormal.z += 1.f;
          else if (objSpaceIntersection.z < -0.4998f) objSpaceNormal.z -= 1.f;

          if (glm::length(objSpaceNormal) == 0)
            cout << glm::to_string(objSpaceIntersection) << endl;

          objSpaceNormal = glm::normalize(objSpaceNormal);

          hit.time = tHit;
          hit.intersection = obj.modelview * objSpaceIntersection; //ray.start + tHit * ray.direction;
          hit.normal = glm::normalize(obj.normalMat * objSpaceNormal);
          hit.mat = &obj.mat;
        }

        void raycastSphere(Ray3D& ray, Ray3D& objSpaceRay, HitRecord& hit, RaycastObj& obj) {
          // Solve quadratic
          float A = objSpaceRay.direction.x * objSpaceRay.direction.x + objSpaceRay.direction.y * objSpaceRay.direction.y + objSpaceRay.direction.z * objSpaceRay.direction.z;
          float B = 2.f * (objSpaceRay.direction.x * objSpaceRay.start.x + objSpaceRay.direction.y * objSpaceRay.start.y + objSpaceRay.direction.z * objSpaceRay.start.z);
          float C = objSpaceRay.start.x * objSpaceRay.start.x + objSpaceRay.start.y * objSpaceRay.start.y + objSpaceRay.start.z * objSpaceRay.start.z - 1.f;

          float radical = B * B - 4.f * A * C;
          // no intersection
          if (radical < 0) return;

          float root = sqrtf(radical);

          float t1 = (-B + root) / (2.f * A);
          float t2 = (B + root) / (2.f * A);

          float tMin = (t1 >= 0 && t2 >= 0) ? min(t1, t2) : max(t1, t2);
          // object is fully behind camera
          if (tMin < 0) return;

          // already hit a closer object
          if (hit.time <= tMin) return;

          hit.time = tMin;
          glm::vec4 objSpaceIntersection = objSpaceRay.start + tMin * objSpaceRay.direction;
          hit.intersection = obj.modelview * objSpaceIntersection; //ray.start + tMin * ray.direction;
          glm::vec3 objSpaceNormal(objSpaceIntersection);
          glm::vec4 normalDir = obj.normalMat * glm::vec4(objSpaceNormal, 0);
          glm::vec3 normal(normalDir);
          hit.normal = glm::normalize(normal);
          hit.mat = &obj.mat;
        }

        void raycast(Ray3D& ray, HitRecord& hit) {

            for(auto& obj : objs)
            {
                // WARN: Might have to normalize direction vec
                Ray3D objSpaceRay(obj.modelviewInverse * ray.start, obj.modelviewInverse * ray.direction);

                if (obj.objType == "box") raycastBox(ray, objSpaceRay, hit, obj);
                else if (obj.objType == "sphere") raycastSphere(ray, objSpaceRay, hit, obj);
            }
        }

        inline static glm::vec3 compMul(const glm::vec3& lhs, const glm::vec3& rhs) {
          return glm::vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
        }

        glm::vec3 shade(HitRecord& hit) {
            glm::vec3& fPosition = hit.intersection;
            glm::vec3& fNormal = hit.normal;
            glm::vec3 fColor(0.f,0.f,0.f);
            glm::vec3 lightVec(0.f,0.f,0.f), viewVec(0.f,0.f,0.f), reflectVec(0.f,0.f,0.f);
            glm::vec3 normalView(0.f,0.f,0.f);
            glm::vec3 ambient(0.f,0.f,0.f), diffuse(0.f,0.f,0.f), specular(0.f,0.f,0.f);
            float nDotL,rDotV;

            for (auto& light : lights)
            {
              if (light.getPosition().w!=0)
                lightVec = glm::normalize(glm::vec3(light.getPosition()) - fPosition);
              else
                lightVec = glm::normalize(-glm::vec3(light.getPosition()));

              /*
              glm::vec3 spotDir = light.getSpotDirection();
              if (glm::length(spotDir) > 0 && -glm::dot(spotDir,lightVec) < light.getSpotCutoff())
              {
                continue;
              }
              */

              glm::vec3 tNormal = fNormal;
              normalView = glm::normalize(tNormal);
              nDotL = glm::dot(normalView,lightVec);

              viewVec = -fPosition;
              viewVec = glm::normalize(viewVec);

              reflectVec = glm::reflect(lightVec,normalView);
              reflectVec = glm::normalize(reflectVec);

              rDotV = glm::dot(reflectVec,viewVec);
              rDotV = max(rDotV,0.0f);

              ambient = compMul(hit.mat->getAmbient(), light.getAmbient());
              diffuse = compMul(hit.mat->getDiffuse(), light.getDiffuse()) * max(nDotL,0.f);
              if (nDotL>0)
                specular = compMul(hit.mat->getSpecular(), light.getSpecular()) * pow(rDotV,max(hit.mat->getShininess(), 1.f));
              fColor = fColor + ambient + diffuse + specular;
            }

            return fColor;
        }

        void raytrace(int width, int height, stack<glm::mat4>& mv) {
            rayHits.resize(height);
            pixelData.resize(height);

            for (int jj = 0; jj < height; ++jj) {
                vector<HitRecord>& hitsRow = rayHits[jj];
                hitsRow.resize(width);
                pixelData[jj].resize(width);

                for (int ii = 0; ii < width; ++ii) {
                    Ray3D ray = screenSpaceToViewSpace(width, height, glm::vec2(ii,height-jj), glm::radians(30.0));

                    raycast(ray, hitsRow[ii]);
                }
            }

            for (int jj = 0; jj < height; ++jj) {
                for (int ii = 0; ii < width; ++ii) {
                    HitRecord& hit = rayHits[jj][ii];
                    if (hit.time < MaxFloat) {
                        pixelData[jj][ii] = shade(hit) * 255.f;
                    }
                }
            }


            // TODO: export to file 'outfileLoc'
            // PPMImageExporter ppmExporter;
            // ppmExporter.export(outfileLoc, width, height, pixelData);
        std::ofstream op(outfileLoc);

        //read in the word P3
        op << "P3" << "\n";
        op << width << " " << height << "\n";
        op << "255\n";

        for (int jj = 0; jj < height; ++jj)
        {
            for (int ii = 0; ii < width; ++ii)
            {
                op << floorf(pixelData[jj][ii].r) << " ";
                op << floorf(pixelData[jj][ii].g) << " ";
                op << floorf(pixelData[jj][ii].b) << std::endl;
            }
        }
        }

        private:
        stack<glm::mat4>& modelview;   
        vector<util::Light> lights;
        // TODO: map<string,util::TextureImage*> textures;

        vector<RaycastObj> objs;

        string outfileLoc;
        vector<vector<HitRecord> > rayHits;
        vector<vector<glm::vec3> > pixelData;
   };
}

#endif
