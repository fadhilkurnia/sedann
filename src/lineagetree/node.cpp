#include "lineagetree/node.h"
#include "lineagetree/utils.h"
#include <cmath>

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
    this->is_leaf = is_leaf;
    this->dim = dim;
    this->max_centroid = max_centroid;
}

float l2_sq_distance(uint32_t dim, float *a, float *b) {
    float dist = 0.0;
    for (int i = 0; i < dim; i++) {
        dist += std::pow(a[i] - b[i], 2);
    }
    return dist;
}

bool Node::insert_vector(float *v) {
    // handle empty node, create a new cluster with a single vector
    if (this->centroids.empty()) {
        if (!this->is_leaf) {
            printf("empty node, but non leaf\n");
            return false;
        }

        // create a new cluster, insert vector v into it
        auto *c = new Cluster(this->dim, 8);
        c->insert_vector(v);

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

    // insert vector v into a cluster whose centroid is the closest
    printf("inserting a vector into cid=%d\n", cid);
    Cluster *c = this->clusters[cid];
    c->insert_vector(v);
    this->centroids[cid] = c->centroid;

    // split the cluster if it is almost full
    if (c->is_almost_full()) {
        printf("current num of vectors: %lu\n", c->vectors.size());
        std::vector<Cluster *> new_clusters = this->split_cluster(c);
        if (this->centroids.size() - 1 + new_clusters.size() <= this->max_centroid) {
            std::vector<float *> tmp_centoids;
            std::vector<Cluster *> tmp_clusters;

            // insert the new clusters into the list of cluster
            // while keeping the relative order
            for (int i = 0; i < this->centroids.size(); i++) {
                if (i == cid) {
                    for (auto &new_cluster: new_clusters) {
                        tmp_clusters.push_back(new_cluster);
                        tmp_centoids.push_back(new_cluster->centroid);
                    }
                    continue;
                }
                tmp_clusters.push_back(this->clusters[i]);
                tmp_centoids.push_back(this->clusters[i]->centroid);
            }
            this->clusters = tmp_clusters;
            this->centroids = tmp_centoids;

        } else {
            this->insert_md_old_centroid = c->centroid;
            this->insert_md_split_centroid_idx = cid;
            // TODO: implement this, maybe in the tree?
            printf("the node is full, need to push-up to the parent. num_centroid=%lu, target_centroid_id=%d\n",
                   this->centroids.size(), cid);
        }
    }

    return true;
}

std::vector<Cluster *> Node::split_cluster(Cluster *c) {
    std::vector<Cluster *> new_clusters;

    // TODO: use k-means to split a cluster into two new clusters
    uint32_t k = 2;
    std::vector<float *> new_centroids;          // new centroids
    std::vector<int> vector_cid(c->vectors.size()); // cid for each vector

    // the default new centroids is the first and the last vector in the cluster
    float *new_c1 = (float *) malloc(this->dim * sizeof(float));
    memcpy(new_c1, c->vectors[0], this->dim);
    new_centroids.push_back(new_c1);
    float *new_c2 = (float *) malloc(this->dim * sizeof(float));
    memcpy(new_c2, c->vectors[c->vectors.size() - 1], this->dim);
    new_centroids.push_back(new_c2);

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

        // TODO: recalculate the centroid for each new cluster
        std::vector<int> num_vec_cluster(k); // #vector in each new cluster
        for (int i = 0; i < c->vectors.size(); i++) {
            update_centroid(
                    num_vec_cluster[vector_cid[i]],
                    this->dim,
                    new_centroids[vector_cid[i]],
                    c->vectors[i]);
        }

        // stop when there is no changes
        if (!any_changes) {
            printf("finish k-means with %d iterations.\n", num_iteration);
            break;
        }
    }

    // initializing the new clusters
    for (float *new_ctr: new_centroids) {
        auto *tmp = new Cluster(this->dim);
        tmp->centroid = new_ctr;
        new_clusters.push_back(tmp);
    }

    // assigning vectors to its new cluster
    for (int i = 0; i < vector_cid.size(); i++) {
        auto tmp_c = new_clusters[vector_cid[i]];
        tmp_c->vectors.push_back(c->vectors[i]);
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