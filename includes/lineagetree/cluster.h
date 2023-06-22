#ifndef LINEAGETREE_CLUSTER_H
#define LINEAGETREE_CLUSTER_H

#include <vector>
#include <cstdint>

// Cluster contains the raw vector
class Cluster {
public:
    uint32_t dim;
    uint32_t max_vector;
    float *centroid;                // centroid is not necessarily a vector element in this cluster.
    std::vector<float *> vectors;    // all the vectors in this cluster.

    explicit Cluster(uint32_t dim = 128, uint32_t max_vector = 8);

    bool insert_vector(float *v);

    bool is_almost_full() const;
};

#endif // LINEAGETREE_CLUSTER_H