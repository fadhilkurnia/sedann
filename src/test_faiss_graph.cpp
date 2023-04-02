#include <iostream>
#include <cstdlib>
#include <random>
#include <fstream>

#include <faiss/IndexNSG.h>

// 64-bit int
using idx_t = faiss::idx_t;

float* read_fvecs(char *filename) {
    std::ifstream reader(filename, std::ios::binary | std::ios::ate);
    size_t fsize = reader.tellg();
    reader.seekg(0, std::ios::beg);

    uint32_t ndims_u32;
    reader.read((char *)&ndims_u32, sizeof(uint32_t));
    reader.seekg(0, std::ios::beg);
    size_t ndims = (size_t) ndims_u32;
    size_t nvecs = fsize / ((ndims + 1) * sizeof(float));
    std::cout << ">> Dataset (" << filename << "): #vector = " << nvecs << ", #dims = " << ndims << std::endl;

    char* buffer = new char[nvecs * (sizeof(uint32_t) + ndims * sizeof(float))];
    reader.read(buffer, nvecs * (sizeof(uint32_t) + ndims * sizeof(float)));
    
    float* data = new float[nvecs * ndims * sizeof(float)];

    for (size_t vid=0; vid < nvecs; vid++) {
        memcpy(
            data + vid * ndims, 
            buffer + vid * (sizeof(uint32_t) + ndims * sizeof(float)),
            ndims * sizeof(float));
    }
    
    reader.close();
    delete[] buffer;
    
    return data;
}

int main() {
    int d = 128;    // dimension
    int nb = 10000; // database size (number of centroids)
    int nq = 10;    // number of search queries

    // read the centroids
    char* filename = "../data/centroids_10k_sift10m.fvecs";
    float* centroids = read_fvecs(filename);

    std::mt19937 rng;
    std::uniform_real_distribution<> distrib(0.0, 255.0);

    float* xb = centroids;
    float* xq = new float[d * nq];

    { 
        printf(">> initialize queries ...\n");
        for (int i = 0; i < nq; i++) {
            for (int j = 0; j < d; j++)
                xq[d * i + j] = distrib(rng);
        }
    }

    printf(">> creating the index\n");
    faiss::IndexNSGFlat index(d, 32);
    index.build_type = 1;   // no need for training
    index.verbose = 1;      // see progress
    index.add(nb, xb);

    int k = 8;
    printf(">> sanity checking, searching 5 first vectors in the dataset\n");
    {
        idx_t* I = new idx_t[k * 5];
        float* D = new float[k * 5];

        index.search(5, xb, k, D, I);

        // print out results
        printf("Q (5 first query)=\n");
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < k; j++)
                printf("%.2f ", xq[i * k + j]);
            printf("\n");
        }
        printf("I (5 first results)=\n");
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < k; j++)
                printf("%5zd ", I[i * k + j]);
            printf("\n");
        }
        printf("D (5 first results)=\n");
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < k; j++)
                printf("%.2f ", D[i * k + j]);
            printf("\n");
        }
    }


    printf(">> search on the index\n");
    {
        idx_t* I = new idx_t[k * nq]; // place to store the results (vector id)
        float* D = new float[k * nq]; // place to store the distances

        index.search(nq, xq, k, D, I);

        // print out results
        printf("I (5 first results)=\n");
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < k; j++)
                printf("%5zd ", I[i * k + j]);
            printf("\n");
        }
        printf("D (5 first results)=\n");
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < k; j++)
                printf("%.2f ", D[i * k + j]);
            printf("\n");
        }
    }


    return 0;
}