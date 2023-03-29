#include <iostream>
#include <thread>
#include <vector>
#include "cores.h"

void initialize();

// to compile   : g++ multicore_main.cpp -I ../include/cores.h -std=c++11 -o main
// to run       : ./main
int main() {

    initialize();

    // get number of physical CPU core
    uint32_t num_cpus = cores();
    std::cout << "Launching " << num_cpus << " threads\n";

    // worker in each CPU core
    auto core_worker = [](uint32_t cid) {
        while (true) {
            printf("thread-%d: i'm alive ...\n ", cid);
        }
    };

    // TODO: pair of queue for entry worker to all cpu worker

    // run all workers in each cpu
    std::vector<std::thread*> workers;
    for (int i = 0; i < num_cpus; i++) {
        auto *t =
            new std::thread(core_worker, i);
        workers.push_back(t);
    }

    // forever loop, until terminated
    for (auto t : workers) {
        (*t).join();
        delete t;
    }

    return 0;
}


// 
void initialize() {

}