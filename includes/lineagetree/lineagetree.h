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
    Node *root;

    // the main constructor
    explicit LineageTree(uint32_t dim = 128);

    // insert_vector adds a single vector v into a cluster node
    // in this lineage tree. Adding a new vector can trigger
    // cluster-node splitting and tree balancing, especially
    // if the target cluster-node becomes full.
    bool insert_vector(float *v);

    // find_target_insert_node return a leaf node which contains
    // approximately the closest centroid
    Node *find_target_insert_node(float *v) const;

    void split_node(Node *n);

    uint32_t get_depth() const;

    uint32_t get_num_nodes() const;

    uint32_t get_num_leaf_nodes() const;

    // print the tree starting from the node n, if n is null the function
    // print from the root of this tree
    void print(Node *n);

    // persist_index store the tree structure into a persistent disk
    Error *persist_index();
};