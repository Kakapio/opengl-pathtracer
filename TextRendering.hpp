#ifndef __TEXTRENDERING_HPP__
#define __TEXTRENDERING_HPP__

#include "sgraph/SGNode.h"
#include "sgraph/SGNodeVisitor.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/TransformNode.h"
#include "sgraph/RotateTransform.h"
#include "sgraph/ScaleTransform.h"
#include "sgraph/TranslateTransform.h"
#include <string>
#include <iostream>

class TextRendering : sgraph::SGNodeVisitor {
  public:
    TextRendering(std::ostream &stream = std::cout);

    virtual void visitGroupNode(sgraph::GroupNode *node);
    virtual void visitLeafNode(sgraph::LeafNode *node);
    virtual void visitTransformNode(sgraph::TransformNode *node);
    virtual void visitScaleTransform(sgraph::ScaleTransform *node);
    virtual void visitTranslateTransform(sgraph::TranslateTransform *node);
    virtual void visitRotateTransform(sgraph::RotateTransform *node);

  private:
    void visitParentNode(sgraph::ParentSGNode *node);
    void visitNode(sgraph::AbstractSGNode *node);

    std::ostream &out;
    int layer = 0;

};

#endif
