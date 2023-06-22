#include "lineagetree/cluster.h"
#include "lineagetree/utils.h"

Cluster::Cluster(uint32_t dim, uint32_t max_vector) {
    this->dim = dim;
    this->max_vector = max_vector;
    this->centroid = nullptr;
}

bool Cluster::insert_vector(float *v) {
    printf("cluster: before insertion: %lu\n", this->vectors.size());
    if (this->vectors.size() == this->max_vector) {
        printf("cluster: the cluster is full\n");
        return false;
    }

    this->vectors.push_back(v);

    if (this->vectors.size() == 1) {
        // update the centroid, since there is only one vector v so
        // the centroid is equal to that single vector v
        this->centroid = (float *) malloc(this->dim * sizeof(float));
        memcpy(this->centroid, v, this->dim);
    } else {
        // update the centroid using weighted average method
        update_centroid(this->vectors.size() - 1,
                        this->dim,
                        this->centroid,
                        v);
    }

    return true;
}

bool Cluster::is_almost_full() const {
    return ((float) this->vectors.size() / (float) this->max_vector) > 0.90;
}
