#ifndef LINEAGETREE_UTILS_H
#define LINEAGETREE_UTILS_H

#include <cstdint>
#include <vector>

// update_centroid updates the centroid, in-place, using weighted average method
static void update_centroid(uint32_t num_prev_vector, uint32_t dim, float *centroid, float *v) {
    // TODO: use SIMD to speed up the calculation below
    auto n = (float) num_prev_vector;
    for (int d = 0; d < dim; d++) {
        centroid[d] = (centroid[d] * n + v[d]) / (n + 1);
    }
}

static float *get_centroid(uint32_t dim, const std::vector<float *> &vectors) {
    if (vectors.empty()) {
        return nullptr;
    }

    // TODO: use SIMD to speed up the calculation below
    auto *new_centroid = (float *) malloc(sizeof(float) * dim);
    memcpy(new_centroid, vectors[0], dim);

    for (int i = 1; i < vectors.size(); ++i) {
        for (int d = 0; d < dim; d++) {
            new_centroid[d] = new_centroid[d] + vectors[i][d];
        }
    }

    float n = (float) vectors.size();
    for (int d = 0; d < dim; d++) {
        new_centroid[d] = new_centroid[d] / n;
    }

    return new_centroid;
}

#endif //LINEAGETREE_UTILS_H
