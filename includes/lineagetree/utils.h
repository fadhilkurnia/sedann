#ifndef LINEAGETREE_UTILS_H
#define LINEAGETREE_UTILS_H

#include <cstdint>

// update_centroid updates the centroid, in-place, using weighted average method
static void update_centroid(uint32_t num_prev_vector, uint32_t dim, float *centroid, float *v) {
    // TODO: use SIMD to speed up the calculation below
    auto n = (float) num_prev_vector;
    for (int d = 0; d < dim; d++) {
        centroid[d] = (centroid[d] * n + v[d]) / (n + 1);
    }
}

#endif //LINEAGETREE_UTILS_H
