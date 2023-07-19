#include "lineagetree/lineagetree.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>


float *read_fvecs(const char *data_filename, uint32_t *dimension,
                  uint32_t *num_vectors);

int32_t *read_ivecs(const char *data_filename, uint32_t *dimension,
                    uint32_t *num_vectors);

int main(int argc, char **argv) {
    const char *data_filename = "../data/sift/sift_base.fvecs";
    uint32_t dimension;
    uint32_t num_vectors;

    float *vectors = read_fvecs(data_filename, &dimension, &num_vectors);
    LineageTree t(dimension);

    // num_vectors = 52;
    printf("== indexing %u vectors ...\n", num_vectors);
    auto begin_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_vectors; i++) {
        uint32_t offset = dimension * i;
        float *v = vectors + offset;
        t.insert_vector(v);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast
            <std::chrono::nanoseconds>(end_time - begin_time);
    printf("== insertion time: %.2f ms\n", duration.count() / (double) 1000000.0);

    printf("=======================\n");
    printf("tree depth: %d\n", t.get_depth());
    printf("tree #node: %d\n", t.get_num_nodes());
    printf("tree #leaf: %d\n", t.get_num_leaf_nodes());
    printf("leafs: "); t.print_leaf_nodes(); printf("\n");

    printf("=======================\n");

    // load the test queries and the ground truth
//    const char *query_filename = "../data/sift/sift_query.fvecs";
//    float *queries = read_fvecs(query_filename, &dimension, &num_vectors);
//    const uint32_t vec_dim = dimension;
//    uint32_t num_queries = num_vectors;
//
//    const char *gt_filename = "../data/sift/sift_groundtruth.ivecs";
//    int32_t *gt_vectors = read_ivecs(gt_filename, &dimension, &num_vectors);
//
//    num_queries = 1;
//    for (int i = 0; i < num_queries; ++i) {
//        auto result = t.approximate_search2(5, queries + i * vec_dim);
//        printf("vector-%d: ", i);
//        for (int j = 0; j < result.size(); ++j) {
//            printf("%d ", result[j]);
//        }
//        printf("\n");
//    }
//
//    for (int i = 0; i < 1; ++i) {
//        printf("vector-%d: ", i);
//        for (int j = 0; j < dimension; ++j) {
//            printf("%d ", gt_vectors[i * dimension + j]);
//        }
//        printf("\n");
//    }

    // test by searching the first five vectors in the dataset
    for (int i = 0; i < 5; ++i) {
        auto result = t.approximate_search2(10, vectors + i * dimension);
        printf("vector-trivial-%d: ", i);
        for (int j = 0; j < result.size(); ++j) {
            printf("%d ", result[j]);
        }
        printf("\n");
    }

    // cleanup
    free(vectors);
    return 0;
}

float *read_fvecs(const char *data_filename, uint32_t *dimension,
                  uint32_t *num_vectors) {
    // open the file
    FILE *data_file = fopen(data_filename, "r");

    if (!data_file) {
        std::cerr << "failed to open data file: " << data_filename << std::endl;
        return nullptr;
    }

    int32_t dim_i32 = 0;
    fread(&dim_i32, 1, sizeof(int32_t), data_file);
    *dimension = (uint32_t) dim_i32;
    fseek(data_file, 0, SEEK_SET);
    struct stat st{};
    fstat(fileno(data_file), &st);
    size_t filesize = st.st_size;

    if (filesize % ((dim_i32 + 1) * 4) != 0) {
        std::cerr << "invalid fvecs file, weird file size: " << data_filename
                  << std::endl;
        return nullptr;
    }
    size_t tmp_num_vectors = filesize / ((dim_i32 + 1) * 4);
    *num_vectors = (uint32_t) tmp_num_vectors;
    printf("reading vectors from %s\n", data_filename);
    printf("dimension    : %d\n", dim_i32);
    printf("filesize     : %zu bytes\n", filesize);
    printf("num vectors  : %zu\n", tmp_num_vectors);

    float *data = new float[dim_i32 * tmp_num_vectors];
    auto *buff_vector = new float[dim_i32 + 1];
    for (size_t i = 0; i < tmp_num_vectors; i++) {
        fread(buff_vector, sizeof(float),
              dim_i32 + 1, data_file);
        memcpy(data + (i * dim_i32), buff_vector + 1, dim_i32);
    }
    delete[] buff_vector;
    fclose(data_file);

    return data;
}

int32_t *read_ivecs(const char *data_filename, uint32_t *dimension,
                    uint32_t *num_vectors) {
    // open the file
    FILE *data_file = fopen(data_filename, "r");

    if (!data_file) {
        std::cerr << "failed to open data file: " << data_filename << std::endl;
        return nullptr;
    }

    int32_t dim_i32 = 0;
    fread(&dim_i32, 1, sizeof(int32_t), data_file);
    *dimension = (uint32_t) dim_i32;
    fseek(data_file, 0, SEEK_SET);
    struct stat st{};
    fstat(fileno(data_file), &st);
    size_t filesize = st.st_size;

    if (filesize % ((dim_i32 + 1) * 4) != 0) {
        std::cerr << "invalid ivecs file, weird file size: " << data_filename
                  << std::endl;
        return nullptr;
    }
    size_t tmp_num_vectors = filesize / ((dim_i32 + 1) * 4);
    *num_vectors = (uint32_t) tmp_num_vectors;
    printf("reading vectors from %s\n", data_filename);
    printf("dimension    : %d\n", dim_i32);
    printf("filesize     : %zu bytes\n", filesize);
    printf("num vectors  : %zu\n", tmp_num_vectors);

    auto *data = new int32_t[dim_i32 * tmp_num_vectors];
    auto *buff_vector = new int32_t[dim_i32];
    for (size_t i = 0; i < tmp_num_vectors; i++) {
        fseek(data_file, 4, SEEK_CUR);
        fread((char *) buff_vector, 1,
              dim_i32 * 4, data_file);
        memcpy(((char *) data) + (i * dim_i32 * 4),
               (char *) buff_vector, dim_i32 * 4);
    }
    delete[] buff_vector;
    fclose(data_file);

    return data;
}
