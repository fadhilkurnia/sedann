#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <vector>
#include "cluster.h"

// LineageClusterNode 
class LineageClusterNode {
public:
    uint32_t id;                // id of this cluster node
    uint32_t dim;               // the dimension of vector in this cluster (e.g: 128)
    uint32_t num_vectors;       // the number of vectors stored in this cluster
    uint32_t pagesize;          // the size of this cluster node = the page size (in bytes)
    uint32_t max_vectors;       // the maximum vectors that a cluster can support
    bool is_leaf;               // is_leaf indicates whether this cluster node is a leaf or not

    float *centroid;            // the centroid of this cluster, not necessarily a real data
    std::vector<float *> data;   // all the vectors in this cluster, empty for non leaf node

    // the main constructor
    explicit LineageClusterNode(uint32_t dim = 128, bool is_leaf = true, uint32_t pagesize = 4096);

    // the destructor ensure to free all the allocated memory
    // for this cluster
    ~LineageClusterNode();

    // insert_vector adds a new vector into this cluster node.
    bool add_vector(float *v);

    // get_cur_capacity returns the current capacity between 0.0 - 1.0
    float get_cur_capacity() const;
};

// Node contains multiple centroids. For a leaf node, each centroid points
// to a cluster, while in a non-leaf node, each centroid points to another
// node.
class Node {
public:
    bool is_leaf;
    uint32_t dim;
    uint32_t max_centroid;
    std::vector<float *> centroids;
    std::vector<Cluster *> clusters;
    std::vector<Node *> children;

    Node *parent;           // pointer to the parent's node
    uint32_t parent_id;     // pointer to the id of parent centroid in parent

    // these variables below are populated when this node splitting
    // a full cluster into two new clusters
    float *insert_md_old_centroid;
    uint32_t insert_md_split_centroid_idx;

    explicit Node(bool is_leaf = true, uint32_t dim = 128, uint32_t max_centroid = 4);

    bool insert_vector(float *v);

    std::vector<Cluster *> split_cluster(Cluster *c);

    bool is_full() const;

    void print();
};
