#pragma once

#include "node.h"
#include "error/error.h"

// checklist:
// [ ] handle insert new vector
// [ ] split cluster node into multiple cluster nodes
// [ ] rebalancing cluster node
// [ ] buffer manager, store some data in disk and memory

class LineageTree {
public:
    uint32_t dim;
    uint32_t depth;
    uint32_t num_nodes;
    Node *root;

    // the main constructor
    explicit LineageTree(uint32_t dim = 128);

    // insert_vector adds a single vector v into a cluster node
    // in this lineage tree. Adding a new vector can trigger
    // cluster-node splitting and tree balancing, especially
    // if the target cluster-node becomes full.
    bool insert_vector(float *v);

    // persist_index store the tree structure into a persistent disk
    Error *persist_index();
};