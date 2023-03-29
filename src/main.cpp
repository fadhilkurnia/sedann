#include <sys/stat.h>

#include <boost/program_options.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

#include "distances.h"

namespace po = boost::program_options;

// ================= FUNCTION HEADERS ==========================================

// process_page calculates the distance between query_vector and all the vectors
// in the page (*vectors).
void process_page(uint32_t pid, float *vectors, uint32_t dim, uint32_t n,
                  float *query_vector, bool is_simd, bool debug);

// vector_distance calculates the vector distance between *a and *b, given that
// both has dim dimension.
double vector_distance(float *a, float *b, uint32_t dim);

// vector_distance_simd is similar as vector_distance, but it uses vectorized
// operation with SIMD.
double vector_distance_simd(float *a, float *b, uint32_t dim);

// =============================================================================

int main(int argc, char **argv) {
    uint32_t args_page_size_kb = 4;
    uint32_t args_num_thread = 1;
    uint32_t args_num_repetition = 1;
    bool args_use_simd = true;
    bool args_debug = false;
    bool args_write_pages = true;
    bool args_memory_only = false;

    const char *data_filename = "../data/sift1m/sift_base.fvecs";
    const char *pages_filename = "../data/sift1m/collection";

    // read and parse the given arguments, put them into variables
    {
        po::options_description desc("Available arguments");
        desc.add_options()("help,h", "print usage message");
        desc.add_options()("page_size,p",
                           po::value<uint32_t>(&args_page_size_kb)->required(),
                           "page size in kb (default: 4KB)");
        desc.add_options()("num_thread,t",
                           po::value<uint32_t>(&args_num_thread),
                           "number of parallel threads");
        desc.add_options()("simd,s", po::value<bool>(&args_use_simd),
                           "using simd or not (default: true)");
        desc.add_options()("write_pages,w", po::value<bool>(&args_write_pages),
                           "write pages into a file or reuse (default: true)");
        desc.add_options()("memory_only,m", po::value<bool>(&args_memory_only),
                           "read pages from file or from memory");
        desc.add_options()("repetition,r",
                           po::value<uint32_t>(&args_num_repetition),
                           "number of repetition in page processing");
        desc.add_options()(
            "debug,d", po::value<bool>(&args_debug),
            "printout text when processing page (default: false)");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        try {
            po::notify(vm);
        } catch (std::exception &e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }

        if (!args_write_pages && !args_memory_only) {
            printf(
                "WARNING: reusing pages file (%s), ensure the file is exist "
                "and the page size is correct! You can run the program with "
                "'--write_pages true' first before running it with "
                "'--write_pages false'\n\n",
                pages_filename);
        }
    }

    // begin reading vectors from file =========================================
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

    // prepare a query for page processing (distance calculation)
    std::uint32_t query_vector_id = 313;
    float *query_vector = new float[dimension];
    fseek(data_file, (dimension + 1) * query_vector_id, SEEK_SET);
    fread(query_vector, sizeof(float), dimension, data_file);
    fseek(data_file, 0, SEEK_SET);

    float *vectors;
    if (args_write_pages || args_memory_only) {
        vectors = new float[dimension * num_vectors];
        auto *buff_vector = new float[dimension + 1];
        for (size_t i = 0; i < num_vectors; i++) {
            fread(buff_vector, sizeof(float), dimension + 1, data_file);
            memcpy(vectors + (i * dimension), buff_vector + 1, dimension);
        }
        delete[] buff_vector;
    }
    fclose(data_file);
    // end reading vectors from file ===========================================

    // begin rewrite into pages ================================================
    size_t page_size_kb = args_page_size_kb;
    size_t page_size = page_size_kb * 1024;
    size_t vectors_per_page = page_size / (dimension * sizeof(float));
    size_t num_pages = num_vectors / vectors_per_page;
    printf("page size    : %zu bytes\n", page_size);
    printf("vector/page  : %zu\n", vectors_per_page);
    size_t wasted_space =
        page_size - (vectors_per_page * dimension * sizeof(float));
    printf("wasted space : %zu byte\n", wasted_space);
    printf("in a page    \n");
    printf("num. of page : %zu\n", num_pages);

    if (args_write_pages) {
        FILE *pages_file = fopen(pages_filename, "w+");
        if (!pages_file) {
            std::cerr << "failed to open collection file: " << pages_filename
                      << std::endl;
            return -1;
        }

        size_t cur_vec_idx = 0;
        std::uint32_t page_id = 0;
        while (cur_vec_idx < num_vectors) {
            size_t file_offset = page_id * page_size;
            fseek(pages_file, file_offset, SEEK_SET);
            fwrite(vectors + cur_vec_idx, sizeof(float),
                   vectors_per_page * dimension, pages_file);
            cur_vec_idx += vectors_per_page;
            page_id++;
        }

        fclose(pages_file);
    }
    // end rewrite into pages ==================================================

    // begin random page processing ============================================

    // random permutation of page access
    std::vector<uint32_t> page_ids;
    for (int i = 0; i < num_pages; ++i) {
        page_ids.push_back(i);
    }
    auto rd = std::random_device{};
    auto rng = std::default_random_engine{rd()};
    std::shuffle(page_ids.begin(), page_ids.end(), rng);
    std::cout << "page access  : ";
    for (int i = 0; i < 5; ++i) {
        page_ids.push_back(i);
        std::cout << page_ids[i] << " ";
    }
    std::cout << " ...\n";

    auto thread_run = [pages_filename, page_size, dimension, vectors_per_page,
                       query_vector, vectors, page_ids, args_use_simd,
                       args_num_repetition, args_debug, args_memory_only](
                          int thread_id, int pid_start_idx, int pid_end_idx) {
        // print out thread information
        if (args_debug) {
            printf("thread-%d (", thread_id);
            std::thread::id this_id = std::this_thread::get_id();
            std::cout << this_id;
            printf("): processing [%d-%d)\n", pid_start_idx, pid_end_idx);
        }

        // using *vectors which is stored in memory
        if (args_memory_only) {
            for (uint32_t r = 0; r < args_num_repetition; r++)
                for (uint32_t pid = pid_start_idx; pid < pid_end_idx; ++pid) {
                    // reading the page from memory
                    uint32_t page_offset_float = pid * vectors_per_page * dimension;
                    float *page = vectors + page_offset_float;

                    // process the page by doing distance calculation
                    process_page(pid, page, dimension, vectors_per_page,
                                 query_vector, args_use_simd, args_debug);
                }

            return;
        }

        FILE *f = fopen(pages_filename, "r");
        if (!f) {
            std::cerr << "thread-" << thread_id
                      << " : failed to open collection file: " << pages_filename
                      << std::endl;
            return;
        }

        char *page = new char[page_size];
        for (uint32_t r = 0; r < args_num_repetition; r++)
            for (uint32_t pid = pid_start_idx; pid < pid_end_idx; ++pid) {
                // reading the page from external file
                uint32_t page_offset = pid * page_size;
                fseek(f, page_offset, SEEK_SET);
                fread(page, sizeof(char), page_size, f);

                // process the page by doing distance calculation
                process_page(pid, (float *)page, dimension, vectors_per_page,
                             query_vector, args_use_simd, args_debug);
            }

        fclose(f);
        delete[] page;
    };

    // init and run workers to process multiple pages
    std::vector<std::thread *> workers;
    uint32_t pages_per_worker = num_pages / args_num_thread;
    std::cout << "num worker   : " << args_num_thread << std::endl;
    std::cout << "pages/worker : " << pages_per_worker << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (int thread_id = 0; thread_id < args_num_thread; thread_id++) {
        int pid_start_idx = thread_id * pages_per_worker;
        int pid_end_idx = thread_id * pages_per_worker + pages_per_worker;
        pid_end_idx =
            pid_end_idx > page_ids.size() ? page_ids.size() : pid_end_idx;

        auto *t =
            new std::thread(thread_run, thread_id, pid_start_idx, pid_end_idx);
        workers.push_back(t);
    }

    // wait for all workers to finish
    for (auto t : workers) {
        (*t).join();
        delete t;
    }
    auto end = std::chrono::high_resolution_clock::now();
    double time_taken =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count();
    std::cout << "\nresults " << std::endl;
    std::cout << " > time              : " << std::fixed << time_taken * 1e-6
              << std::setprecision(9);
    std::cout << "  ms " << std::endl;
    std::cout << " > proc throughput   : " << std::fixed
              << (num_vectors * args_num_repetition) / (time_taken * 1e-9)
              << std::setprecision(9);
    std::cout << "  calculation/s " << std::endl;

    // end random page processing ==============================================

    if (args_write_pages) {
        delete[] vectors;
    }

    return 0;
}

void process_page(uint32_t pid, float *vectors, uint32_t dim, uint32_t n,
                  float *query_vector, bool is_simd, bool debug) {
    auto start = std::chrono::high_resolution_clock::now();
    double tmp = 0.0;
    for (int i = 0; i < n; ++i) {
        uint32_t target_vector_idx = i;
        if (is_simd)
            tmp += vector_distance_simd(
                query_vector, vectors + (target_vector_idx * dim), dim);
        else
            tmp += vector_distance(query_vector,
                                   vectors + (target_vector_idx * dim), dim);
    }
    if (debug) {
        auto end = std::chrono::high_resolution_clock::now();
        double time_taken =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                .count();
        std::cout << "processing page: " << pid << std::endl;
        std::cout << "  use simd        : " << is_simd << std::endl;
        std::cout << "  time            : " << std::fixed << time_taken * 1e-3
                  << std::setprecision(9);
        std::cout << "  µs " << std::endl;
        std::cout << "  calc throughput : " << n / (time_taken * 1e-9)
                  << " vec/s" << std::endl;
        std::cout << "  calc result     : " << tmp << std::endl << std::endl;
    }
}

double vector_distance(float *a, float *b, uint32_t dim) {
    double dist = 0.0;
    for (int i = 0; i < dim; ++i) {
        double tmp = (a[i] - b[i]);
        dist += tmp * tmp;
    }
    return dist;
}

double vector_distance_simd(float *a, float *b, uint32_t dim) {
    return fvec_L2sqr_avx(a, b, dim);
}