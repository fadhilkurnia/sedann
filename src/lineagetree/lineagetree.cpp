#include "lineagetree/lineagetree.h"

LineageTree::LineageTree(uint32_t dim) {
    printf("LineageTree constructor\n");
    
    this->dim = dim;
    this->depth = 0;
    this->num_nodes = 0;
    this->root = NULL;
}

bool LineageTree::add_vector(float* v) {
    printf("adding a new vector %d\n", this->dim);
    
    if (this->root == NULL) {
        this->root = new LineageClusterNode(this->dim, true);
    }

    this->root->add_vector(v);

    return true;
}