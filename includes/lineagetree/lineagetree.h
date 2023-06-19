#include "node.h"
#include "error/error.h"

// checklist:
// [ ] handle insert new vector
// [ ] split cluster node into multiple cluster nodes
// [ ] rebalancing cluster node
// [ ] buffer manager, store some data in disk and memory

class LineageTree {
    public:
    uint32_t dim;
    uint32_t depth;
    uint32_t num_nodes;
    LineageClusterNode* root;

    // the main constructor
    LineageTree(uint32_t dim = 128);

    // add_vector adds a single vector v into a cluster node
    // in this lineage tree. Adding a new vector can trigger
    // cluster-node splitting and tree rebalancing, especially
    // if the target cluster-node becomes full.
    bool add_vector(float* v);
    
    // persist_index store the tree structure into a persistent disk
    Error* persist_index();

    private:
    void split_cluster();
};