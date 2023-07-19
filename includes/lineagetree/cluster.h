#ifndef LINEAGETREE_CLUSTER_H
#define LINEAGETREE_CLUSTER_H

#include <vector>
#include <cstdint>

// Cluster contains the raw vector
class Cluster {
public:
    uint32_t dim;
    uint32_t max_vector;
    float *centroid;                 // centroid is not necessarily a vector element in this cluster.
    std::vector<float *> vectors;    // all the vectors in this cluster.
    std::vector<uint32_t> vid;       // the corresponding id of each vector.


    explicit Cluster(uint32_t dim = 128, uint32_t max_vector = 8);

    bool insert_vector(uint32_t vid, float *v);

    // is_almost_full return true if the cluster
    // already store 90% of the maximum capacity
    bool is_almost_full() const;
};

#endif // LINEAGETREE_CLUSTER_H