#ifndef _RAYCASTRENDERER_H_
#define _RAYCASTRENDERER_H_

#include "Light.h"
#include "Material.h"
#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TextureImage.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
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
#include "glm/gtx/wrap.hpp"

#include <glm/glm.hpp>
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
            util::TextureImage* texture;

            RaycastObj(string& type, glm::mat4 mv, util::Material& mat, util::TextureImage* texture):
              objType(type),
              modelview(mv),
              modelviewInverse(glm::inverse(mv)),
              normalMat(glm::inverseTranspose(mv)),
              mat(mat),
              texture(texture) {
              /*
              cout << objType << endl;
              cout << glm::to_string(modelview) << endl;
              cout << glm::to_string(modelviewInverse) << endl;
              cout << glm::to_string(normalMat) << endl;
              cout << endl;
              */
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
        RaycastRenderer(map<string,util::TextureImage*>& textures, stack<glm::mat4>& mv, vector<util::Light>& lights, string outfileLoc) 
            : textures(textures),
              modelview(mv),
              lights(lights),
              outfileLoc(outfileLoc) {
          for (auto& light : this->lights) {
            glm::vec4 lightPosition = light.getPosition();
            // lightPosition.z = -lightPosition.z;
            lightPosition = modelview.top() * lightPosition;
            // cout << glm::to_string(lightPosition) << endl;
            light.setPosition(lightPosition);
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
            if (modelType != "box" && modelType != "sphere"
            && modelType != "cylinder" && modelType != "cone") return;

            glm::mat4 mv = modelview.top();
            util::Material mat = leafNode->getMaterial();
            util::TextureImage* texture = textures[leafNode->getTextureName()];

            objs.emplace_back(modelType, mv, mat, texture);
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

        bool intersectsCylinderCaps(float& tMin, float& tMax, float start, float dir) {
            float t1 = -start;
            float t2 = (1.f - start);

            if (dir == 0) {
                // no intersection
                if (start < 0 || start > 1.f) return false;

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
          objSpaceIntersection.w = 1.f;

          glm::vec4 objSpaceNormal(0.f,0.f,0.f,0.f);
          if (objSpaceIntersection.x > 0.4998f) objSpaceNormal.x += 1.f;
          else if (objSpaceIntersection.x < -0.4998f) objSpaceNormal.x -= 1.f;

          if (objSpaceIntersection.y > 0.4998f) objSpaceNormal.y += 1.f;
          else if (objSpaceIntersection.y < -0.4998f) objSpaceNormal.y -= 1.f;

          if (objSpaceIntersection.z > 0.4998f) objSpaceNormal.z += 1.f;
          else if (objSpaceIntersection.z < -0.4998f) objSpaceNormal.z -= 1.f;

          objSpaceNormal = glm::normalize(objSpaceNormal);

          hit.time = tHit;
          hit.intersection = obj.modelview * objSpaceIntersection; //ray.start + tHit * ray.direction;
          hit.normal = glm::normalize(glm::vec3(obj.normalMat * objSpaceNormal));
          hit.mat = &obj.mat;
          hit.texture = obj.texture;
        }

        void raycastSphere(Ray3D &objSpaceRay, HitRecord &hit, RaycastObj &obj) {
          // Solve quadratic
          float A = objSpaceRay.direction.x * objSpaceRay.direction.x + objSpaceRay.direction.y * objSpaceRay.direction.y + objSpaceRay.direction.z * objSpaceRay.direction.z;
          float B = 2.f * (objSpaceRay.direction.x * objSpaceRay.start.x + objSpaceRay.direction.y * objSpaceRay.start.y + objSpaceRay.direction.z * objSpaceRay.start.z);
          float C = objSpaceRay.start.x * objSpaceRay.start.x + objSpaceRay.start.y * objSpaceRay.start.y + objSpaceRay.start.z * objSpaceRay.start.z - 1.f;

          float radical = B * B - 4.f * A * C;
          // no intersection
          if (radical < 0) return;

          float root = sqrtf(radical);

          float t1 = (-B - root) / (2.f * A);
          float t2 = (-B + root) / (2.f * A);

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
          hit.texture = obj.texture;

          float phi = asinf(objSpaceIntersection.y);
          float theta = atan2f(objSpaceIntersection.z, objSpaceIntersection.x);

          // Gives us 270 instead of -90. No negative angles.
          theta = -theta;
          if (theta < 0.0f) {
              theta += glm::radians(360.0f);
          }

          hit.texCoord = glm::vec2(theta / glm::radians(360.0f),
                                   phi / glm::radians(180.0f) + 0.5f);
        }

        void raycastCylinder(Ray3D& ray, Ray3D& objSpaceRay, HitRecord& hit, RaycastObj& obj) {
            float tyMin, tyMax;
            if (!intersectsCylinderCaps(tyMin, tyMax, objSpaceRay.start.y, objSpaceRay.direction.y))
              return;
            
            float A = objSpaceRay.direction.x * objSpaceRay.direction.x
                    + objSpaceRay.direction.z * objSpaceRay.direction.z;
            float B = 2 * objSpaceRay.start.x * objSpaceRay.direction.x
                    + 2 * objSpaceRay.start.z * objSpaceRay.direction.z;
            float C = objSpaceRay.start.x * objSpaceRay.start.x
                    + objSpaceRay.start.z * objSpaceRay.start.z - 1.f;

            float radical = B * B - 4.f * A * C;
            // no intersection
            if (radical < 0) return;

            float root = sqrtf(radical);

            float t1 = (-B - root) / (2.f * A);
            float t2 = (-B + root) / (2.f * A);

            float tcMin = min(t1, t2), tcMax = max(t1, t2);

            float tMin = max(tyMin, tcMin), tMax = min(tyMax, tcMax);
            // no intersection
            if (tMax < tMin) return;
            
            float tHit = (tMin >= 0 && tMax >= 0) ? min(tMin, tMax) : max(tMin, tMax);

            // object is fully behind camera
            if (tHit < 0) return;

            // already hit a closer object
            if (hit.time <= tHit) return;

            hit.time = tHit;
            glm::vec4 objSpaceIntersection = objSpaceRay.start + tMin * objSpaceRay.direction;
            hit.intersection = obj.modelview * objSpaceIntersection;
            
            glm::vec3 normal;
            if (objSpaceIntersection.y < 0.001f) normal = glm::vec3(0.f, -1.f, 0.f);
            else if (objSpaceIntersection.y > 0.999f) normal = glm::vec3(0.f, 1.f, 0.f);
            else {
              glm::vec3 objSpaceNormal(objSpaceIntersection);
              objSpaceNormal.y = 0;
              normal = objSpaceNormal;
            }
            normal = obj.normalMat * glm::vec4(normal, 0);
            hit.normal = glm::normalize(normal);
            hit.mat = &obj.mat;
            hit.texture = obj.texture;
            /*

            float phi = asinf(objSpaceIntersection.y);
            float theta = atan2f(objSpaceIntersection.z, objSpaceIntersection.x);

            // Gives us 270 instead of -90. No negative angles.
            theta = -theta;
            if (theta < 0.0f) {
              theta += glm::radians(360.0f);
            }

            hit.texCoord = glm::vec2(theta / glm::radians(360.0f),
                phi / glm::radians(180.0f) + 0.5f);
                */
        }

        void raycastCone(Ray3D& ray, Ray3D& objSpaceRay, HitRecord& hit, RaycastObj& obj) {
            float tyMin, tyMax;
            if (!intersectsCylinderCaps(tyMin, tyMax, objSpaceRay.start.y, objSpaceRay.direction.y))
              return;

            float yStart = objSpaceRay.start.y - 1;

            float A = objSpaceRay.direction.x * objSpaceRay.direction.x
                    + objSpaceRay.direction.z * objSpaceRay.direction.z
                    - objSpaceRay.direction.y * objSpaceRay.direction.y;

            float B = 2.f * objSpaceRay.start.x * objSpaceRay.direction.x
                      + 2.f * objSpaceRay.start.z * objSpaceRay.direction.z
                      - 2.f * objSpaceRay.direction.y * yStart;

            float C = objSpaceRay.start.x * objSpaceRay.start.x
                      + objSpaceRay.start.z * objSpaceRay.start.z
                      - yStart * yStart;

            float radical = B * B - 4.f * A * C;
            // no intersection
            if (radical < 0) return;

            float root = sqrtf(radical);

            float t1 = (-B - root) / (2.f * A);
            float t2 = (-B + root) / (2.f * A);

            float tcMin = min(t1, t2), tcMax = max(t1, t2);
            float tMin, tMax;

            if (tcMin < 0) {
              if (tcMax < tyMin || tcMax > tyMax) return;
              tMin = tcMax;
              tMax = tyMax;
            } else {
              if (tcMax < tyMin) return;
              if (tyMax < tcMin) return;

              tMin = max(tyMin, tcMin);
              tMax = min(tyMax, tcMax);

              if (tMin == tyMin && tMax == tyMax) return;
            }
            //float tMin = tcMin, tMax = tcMax;
            // no intersection
            if (tMax < tMin) return;
            
            float tHit = (tMin >= 0 && tMax >= 0) ? min(tMin, tMax) : max(tMin, tMax);

            // object is fully behind camera
            if (tHit < 0) return;

            // already hit a closer object
            if (hit.time <= tHit) return;

            hit.time = tHit;
            glm::vec4 objSpaceIntersection = objSpaceRay.start + tMin * objSpaceRay.direction;
            hit.intersection = obj.modelview * objSpaceIntersection;

            glm::vec3 normal;
            if (objSpaceIntersection.y < 0.001f) normal = glm::vec3(0.f, -1.f, 0.f);
            else {
              glm::vec3 objSpaceNormal(objSpaceIntersection);
              objSpaceNormal.y = 0;
              normal = glm::normalize(objSpaceNormal);
              
              normal.y = sqrtf(normal.x * normal.x + normal.z * normal.z);
            }
            normal = obj.normalMat * glm::vec4(normal, 0);
            hit.normal = glm::normalize(normal);
            hit.mat = &obj.mat;
            hit.texture = obj.texture;
        }

        void raycast(Ray3D& ray, HitRecord& hit) {

            for(auto& obj : objs)
            {
                // WARN: Might have to normalize direction vec
                Ray3D objSpaceRay(obj.modelviewInverse * ray.start, obj.modelviewInverse * ray.direction);

                if (obj.objType == "box") raycastBox(ray, objSpaceRay, hit, obj);
                else if (obj.objType == "sphere") raycastSphere(objSpaceRay, hit, obj);
                else if (obj.objType == "cylinder") raycastCylinder(ray, objSpaceRay, hit, obj);
                else if (obj.objType == "cone") raycastCone(ray, objSpaceRay, hit, obj);
            }
            if (hit.time == MaxFloat) return;

            hit.reflection = glm::reflect(glm::vec3(ray.direction), hit.normal);
        }

        inline static glm::vec3 compMul(const glm::vec3& lhs, const glm::vec3& rhs) {
          return glm::vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
        }

        glm::vec3 showNormals(HitRecord& hit) {
            return (hit.normal + glm::vec3(1., 1., 1.)) * 0.5f * 255.f;
        }

        glm::vec3 shade(HitRecord& hit, int maxBounces = 0) {
            glm::vec3& fPosition = hit.intersection;
            glm::vec3& fNormal = hit.normal;
            glm::vec3 fColor(0.f,0.f,0.f);
            glm::vec3 lightVec(0.f,0.f,0.f), viewVec(0.f,0.f,0.f), reflectVec(0.f,0.f,0.f);
            glm::vec3 normalView(0.f,0.f,0.f);
            glm::vec3 ambient(0.f,0.f,0.f), diffuse(0.f,0.f,0.f), specular(0.f,0.f,0.f);
            float nDotL,rDotV;

            glm::vec3 absorbColor(0.f,0.f,0.f), reflectColor(0.f,0.f,0.f), transparencyColor(0.f,0.f,0.f);

            for (auto& light : lights)
            {
              if (light.getPosition().w!=0)
                lightVec = glm::vec3(light.getPosition()) - fPosition;
              else
                lightVec = -glm::vec3(light.getPosition());

              // Shoot ray towards light source, any hit means shadow.
              Ray3D rayToLight(fPosition, lightVec);
              // Need 'skin' width to avoid hitting itself.
              rayToLight.start += 0.1f * glm::normalize(rayToLight.direction);
              HitRecord shadowcastHit;

              raycast(rayToLight, shadowcastHit);

              lightVec = glm::normalize(lightVec);

              glm::vec3 spotDir = light.getSpotDirection();
              if (glm::length(spotDir) > 0 && -glm::dot(spotDir,lightVec) < light.getSpotCutoff())
              {
                continue;
              }

              glm::vec3 tNormal = fNormal;
              normalView = glm::normalize(tNormal);
              nDotL = glm::dot(normalView,lightVec);

              viewVec = -fPosition;
              viewVec = glm::normalize(viewVec);

              reflectVec = glm::reflect(-lightVec,normalView);
              reflectVec = glm::normalize(reflectVec);

              rDotV = glm::dot(reflectVec,viewVec);
              rDotV = max(rDotV,0.0f);

              ambient = compMul(hit.mat->getAmbient(), light.getAmbient());
              // Object cannot directly see the light
              if (shadowcastHit.time >= 1.f || shadowcastHit.time < 0) {
                diffuse = compMul(hit.mat->getDiffuse(), light.getDiffuse()) * max(nDotL,0.f);
                if (nDotL>0)
                  specular = compMul(hit.mat->getSpecular(), light.getSpecular()) * pow(rDotV,max(hit.mat->getShininess(), 1.f));
              }
              else {
                diffuse = {0., 0., 0.};
                specular = {0., 0., 0.};
              }
              absorbColor = absorbColor + ambient + diffuse + specular;
            }

            glm::vec3 texColor = glm::vec3(hit.texture->getColor(hit.texCoord.s, hit.texCoord.t)) / 255.f;

            absorbColor = compMul(absorbColor, texColor);
            //absorbColor = texColor;
            //absorbColor = glm::vec4(hit.texCoord.s,hit.texCoord.t,0,1);

            if (maxBounces > 0 && hit.mat->getAbsorption() <= 0.999f) {
              Ray3D reflectionRay(fPosition, hit.reflection);
              reflectionRay.start += glm::normalize(reflectionRay.direction) * 0.001f;
              HitRecord reflectionHit;
              raycast(reflectionRay, reflectionHit);

              if (reflectionHit.time < MaxFloat)
                reflectColor = shade(reflectionHit, maxBounces - 1);

              fColor = absorbColor * hit.mat->getAbsorption()
                       + reflectColor * hit.mat->getReflection()
                       + transparencyColor * hit.mat->getTransparency();
            }
            else {
              fColor = absorbColor;
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
                        if (jj == 350 && ii == 400) {
                          pixelData[jj][ii] = shade(hit, 10) * 255.f;
                        }
                        pixelData[jj][ii] = shade(hit, 10) * 255.f;
                        // glm::vec3(255.0f, 255.0f, 255.0f);
                        // pixelData[jj][ii] = showNormals(hit);
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
                op << min(255, (int)floorf(pixelData[jj][ii].r)) << " ";
                op << min(255, (int)floorf(pixelData[jj][ii].g)) << " ";
                op << min(255, (int)floorf(pixelData[jj][ii].b)) << std::endl;
            }
        }
        }

        private:
        map<string,util::TextureImage *>& textures;
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
