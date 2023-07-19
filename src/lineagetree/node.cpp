#include "lineagetree/node.h"
#include "lineagetree/utils.h"
#include <cmath>

const uint32_t CENTROID_MAX_VECTOR = 8;

LineageClusterNode::LineageClusterNode(uint32_t dim, bool is_leaf, uint32_t pagesize) {
    printf("ClusterNode constructor\n");

    this->id = 0;           // TODO: use current time or random number as ID
    this->pagesize = pagesize;
    this->dim = dim;
    this->num_vectors = 0;
    this->is_leaf = is_leaf;
    this->centroid = nullptr;

    // calculate the max number of vectors
    this->max_vectors = pagesize / (dim * sizeof(float));
    printf("max vector: %d\n", this->max_vectors);
    printf("sizeof(float): %lu\n", sizeof(float));
}

LineageClusterNode::~LineageClusterNode() {
    printf("ClusterNode destructor\n");

    if (this->centroid != nullptr) {
        free(this->centroid);
    }
}

bool LineageClusterNode::add_vector(float *v) {
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

float LineageClusterNode::get_cur_capacity() const {
    return (float) this->num_vectors / (float) this->max_vectors;
}

Node::Node(bool is_leaf, uint32_t dim, uint32_t max_centroid) {
    assert(max_centroid > 1);
    assert(dim > 0);

    this->is_leaf = is_leaf;
    this->dim = dim;
    this->max_centroid = max_centroid;
}

bool Node::insert_vector(uint32_t vid, float *v) {
    // handle empty node, create a new cluster with a single vector
    if (this->centroids.empty()) {
        if (!this->is_leaf) {
            // TODO: change printf with a logging library
            printf("error: found a non-leaf empty node\n");
            return false;
        }

        // create a new cluster, insert vector v into it
        auto *c = new Cluster(this->dim, CENTROID_MAX_VECTOR);
        c->insert_vector(vid, v);

        // insert a new centroid to this node
        this->centroids.push_back(c->centroid);
        this->clusters.push_back(c);

        return true;
    }

    // find the closest centroid
    int cid = 0;
    float min_dist = l2_sq_distance(this->dim, this->centroids[0], v);
    for (int i = 1; i < this->centroids.size(); i++) {
        float tmp_dist = l2_sq_distance(this->dim, this->centroids[i], v);
        if (tmp_dist < min_dist) {
            min_dist = tmp_dist;
            cid = i;
        }
    }

    // insert vector v into a cluster whose centroid is the closest (cid)
    // printf("node: inserting a vector into cid=%d\n", cid);
    Cluster *c = this->clusters[cid];
    c->insert_vector(vid, v);
    this->centroids[cid] = c->centroid;

    // split the cluster if it is almost full
    if (c->is_almost_full()) {
        // printf("node: current num of vectors: %lu\n", c->vectors.size());
        std::vector<Cluster *> new_clusters = this->split_cluster(c);

        if (this->centroids.size() - 1 + new_clusters.size() <= this->max_centroid) {
            // the newly created centroids can be placed in this node since
            // this node still have spaces

            std::vector<float *> tmp_centroids;
            std::vector<Cluster *> tmp_clusters;

            // insert the new clusters into the list of cluster
            // while keeping the relative order
            for (int i = 0; i < this->centroids.size(); i++) {
                if (i == cid) {
                    for (auto &new_cluster: new_clusters) {
                        tmp_clusters.push_back(new_cluster);
                        tmp_centroids.push_back(new_cluster->centroid);
                    }
                    continue;
                }
                tmp_clusters.push_back(this->clusters[i]);
                tmp_centroids.push_back(this->clusters[i]->centroid);
            }
            this->clusters = tmp_clusters;
            this->centroids = tmp_centroids;

        } else {
            // the newly created centroids can not be placed in this node since
            // this node is already full, thus the Tree class need to split
            // the node. Here, we set some metadata to help the Tree splitting
            // the node.

            this->insert_md_old_centroid = c->centroid;
            this->insert_md_split_centroid_idx = cid;

            // TODO: we should use new_clusters created from Node::split_cluster(), we already did k-means there, not using the result is wasteful! Alternatively only do k-means if we know for sure that we will still have some places
            // TODO: implement this, maybe in the tree class?
            // printf("the node is full, need to push-up to the parent. num_centroid=%lu, target_centroid_id=%d\n",
            //       this->centroids.size(), cid);
        }
    }

    return true;
}

std::vector<Cluster *> Node::split_cluster(Cluster *c) {
    std::vector<Cluster *> new_clusters;

    // TODO: use efficient k-means to split a cluster into two new clusters
    uint32_t k = 2;
    std::vector<float *> new_centroids;                // new centroids
    std::vector<int> vector_cid(c->vectors.size()); // cid for each vector

    // TODO: use k instead of 2
    // the default new centroids is the first and the last vector in the cluster
    float *new_c1 = (float *) malloc(this->dim * sizeof(float));
    memcpy(new_c1, c->vectors[0], this->dim);
    new_centroids.push_back(new_c1);
    float *new_c2 = (float *) malloc(this->dim * sizeof(float));
    memcpy(new_c2, c->vectors[c->vectors.size() - 1], this->dim);
    new_centroids.push_back(new_c2);

    // TODO: assign to k clusters, instead of 2 clusters
    // initial assignment, split the vector data equally into two clusters
    for (int i = 0; i < c->vectors.size(); i++) {
        vector_cid[i] = 0;
        if (i > c->vectors.size() / 2) vector_cid[i] = 1;
    }

    uint32_t num_iteration = 0;
    while (true) {
        bool any_changes = false;
        num_iteration++;

        for (int i = 0; i < c->vectors.size(); i++) {
            float *cur_v = c->vectors[i];

            // for each vector, find the closest centroid to it, starting
            // from the first centroid
            float min_dist = l2_sq_distance(this->dim,
                                            cur_v,
                                            new_centroids[0]);

            for (int j = 1; j < new_centroids.size(); ++j) {
                float tmp_dist = l2_sq_distance(this->dim,
                                                cur_v,
                                                new_centroids[j]);

                if (tmp_dist < min_dist) {
                    min_dist = tmp_dist;
                    if (vector_cid[i] != j) {
                        vector_cid[i] = j;
                        any_changes = true;
                    }
                }
            }
        }

        std::vector<int> num_vec_cluster(k); // #vector in each new cluster
        // reset the centroid in each cluster, before we recalculate it later
        for (auto & new_centroid : new_centroids) {
            for (int d = 0; d < this->dim; ++d) {
                new_centroid[d] = 0.0;
            }
        }
        // recalculate the new centroid for each cluster
        for (int i = 0; i < c->vectors.size(); i++) {
            update_centroid(
                    num_vec_cluster[vector_cid[i]],
                    this->dim,
                    new_centroids[vector_cid[i]],
                    c->vectors[i]);
            num_vec_cluster[vector_cid[i]]++;
        }

        // stop when there is no changes
        if (!any_changes) {
            // printf("finish k-means with %d iterations.\n", num_iteration);
            break;
        }
    }

    // initializing the new clusters
    for (float *new_ctr: new_centroids) {
        auto *tmp = new Cluster(this->dim, CENTROID_MAX_VECTOR);
        tmp->centroid = new_ctr;
        new_clusters.push_back(tmp);
    }

    // assigning vectors to its new cluster
    for (int i = 0; i < vector_cid.size(); i++) {
        auto tmp_c = new_clusters[vector_cid[i]];
        tmp_c->vectors.push_back(c->vectors[i]);
        tmp_c->vid.push_back(c->vid[i]);
    }

    // throw error when a new cluster is empty
    for (auto nc: new_clusters) {
        if (nc->vectors.empty()) {
            printf("ERROR: split produces an empty cluster\n");
        }
    }

    return new_clusters;
}

bool Node::is_full() const {
    return this->centroids.size() == this->max_centroid;
}

void Node::print() {
    printf("[node %p]\n", this);
    printf(" is_leaf   : %d\n", this->is_leaf);
    printf(" max_ctroid: %u\n", this->max_centroid);
    printf(" #centroid : %ld\n", this->centroids.size());
    printf(" #clusters : %ld\n", this->clusters.size());
    printf(" #children : %ld\n", this->children.size());
    printf(" parent    : %p\n", this->parent);
    printf(" parent_id : %u\n", this->parent_id);
    printf("\n");
}
