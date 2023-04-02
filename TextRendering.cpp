#include "TextRendering.hpp"

#include <iostream>
#include <vector>

using namespace std;

TextRendering::TextRendering(ostream &stream): out(stream) { }

void TextRendering::visitParentNode(sgraph::ParentSGNode *node) {
    visitNode(node);
    vector<sgraph::SGNode *> nodes = node->getChildren();
    layer++;

    for (auto &child : nodes) {
        child->accept(this);
    }

    layer--;
}

void TextRendering::visitNode(sgraph::AbstractSGNode *node) {
    if (layer != 0) {
        out << string((layer - 1) * 4, ' ');
        out << "  - ";
    }
    out << node->getName() << endl;
}

void TextRendering::visitGroupNode(sgraph::GroupNode *node) {
    visitParentNode(node);
}
void TextRendering::visitLeafNode(sgraph::LeafNode *node) {
    visitNode(node);
}

void TextRendering::visitTransformNode(sgraph::TransformNode *node) {
    visitParentNode(node);
}
void TextRendering::visitScaleTransform(sgraph::ScaleTransform *node) {
    visitParentNode(node);
}
void TextRendering::visitTranslateTransform(sgraph::TranslateTransform *node) {
    visitParentNode(node);
}
void TextRendering::visitRotateTransform(sgraph::RotateTransform *node) {
    visitParentNode(node);
}
