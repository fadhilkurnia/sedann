#include "lineagetree/node.h"
#include <iostream>

LineageClusterNode::LineageClusterNode(uint32_t dim, bool is_leaf, uint32_t pagesize) {
    printf("ClusterNode constructor\n");

    this->id = 0;           // TODO: use current time or random number as ID
    this->pagesize = pagesize;
    this->dim = dim;
    this->num_vectors = 0;
    this->is_leaf = is_leaf;
    this->centroid = NULL;

    // calculate the max number of vectors
    this->max_vectors = pagesize / (dim * sizeof(float));
    printf("max vector: %d\n", this->max_vectors);
    printf("sizeof(float): %lu\n", sizeof(float));
}

LineageClusterNode::~LineageClusterNode() {
    printf("ClusterNode destructor\n");
    
    if (this->centroid != NULL) {
            free(this->centroid);
    }
}

bool LineageClusterNode::add_vector(float* v) {
    printf("ClusterNode, adding a new vector\n");
    
    this->data.push_back(v);
    this->num_vectors++;

    if (this->num_vectors == 0) {
        this->centroid = v;
    }

    // TODO: handle if the cluster is 80% full
    if (this->get_cur_capacity() >= 0.8) {
        printf("cluster is almost full!\n");
    }

    if (this->get_cur_capacity() >= 1.0) {
        printf("cluster is already full!\n");
    }
    

    return true;
}

float LineageClusterNode::get_cur_capacity() {
    return (float) this->num_vectors / (float) this->max_vectors;
}
