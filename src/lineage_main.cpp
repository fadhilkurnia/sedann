#include "lineagetree/lineagetree.h"
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>

int main(int argc, char **argv) {
    const char *data_filename = "../data/sift/sift_base.fvecs";

    // ===== reading vectors from file ===============================================
    FILE *data_file = fopen(data_filename, "r");
    int32_t dimension;

    if (!data_file) {
        std::cerr << "failed to open data file: " << data_filename << std::endl;
        return -1;
    }

    fread(&dimension, 1, sizeof(int32_t), data_file);
    fseek(data_file, 0, SEEK_SET);
    struct stat st {};
    fstat(fileno(data_file), &st);
    size_t filesize = st.st_size;

        if (filesize % ((dimension + 1) * 4) != 0) {
        std::cerr << "invalid fvecs file, weird file size: " << data_filename
                  << std::endl;
        return -1;
    }
    size_t num_vectors = filesize / ((dimension + 1) * 4);
    printf("reading vectors from %s\n", data_filename);
    printf("dimension    : %d\n", dimension);
    printf("filesize     : %zu bytes\n", filesize);
    printf("num vectors  : %zu\n", num_vectors);

    float *vectors;
    vectors = new float[dimension * num_vectors];
    auto *buff_vector = new float[dimension + 1];
    for (size_t i = 0; i < num_vectors; i++) {
        fread(buff_vector, sizeof(float), dimension + 1, data_file);
        memcpy(vectors + (i * dimension), buff_vector + 1, dimension);
    }
    delete[] buff_vector;
    fclose(data_file);
    // end reading vectors from file =================================================


    LineageTree t(dimension);
    
    for (int i = 0; i < 40; i++) {
        uint32_t offset = dimension * i;
        float* v = vectors + offset;
         t.insert_vector(v);
    }

    free(vectors);
    return 0;
}
