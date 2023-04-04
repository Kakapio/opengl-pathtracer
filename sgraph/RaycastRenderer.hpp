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
#include <stack>
#include <iostream>
#include <unordered_map>
#include "../Ray3D.hpp"

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
        RaycastRenderer(stack<glm::mat4>& mv,map<string,util::ObjectInstance *>& os) 
            : modelview(mv)
            , objects(os) {
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

            string name = leafNode->getInstanceOf();
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
        Ray3D worldToScreenSpace(float width, float height, glm::vec2 pos, float angle) {
            float halfWidth = width / 2.0f;
            float halfHeight = height / 2.0f;
            float aspect = width / height;

            //TODO: use camera position as start?
            Ray3D out = Ray3D(glm::vec3(0,0,0), glm::vec3(pos.x - halfWidth, pos.y - halfHeight,
                                                          -1 * (halfHeight / tan(angle))));

            return out;
        }

        private:
        stack<glm::mat4>& modelview;    
        map<string,util::ObjectInstance *> objects;
        // TODO: map<string,util::TextureImage*> textures;
        unordered_map<string,glm::mat4> modelviewMap;
        unordered_map<string,glm::mat4> normalmatrixMap;

   };
}

#endif
