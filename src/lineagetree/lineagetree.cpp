#include "lineagetree/lineagetree.h"
#include <stack>

LineageTree::LineageTree(uint32_t dim) {
    printf("LineageTree constructor\n");

    this->dim = dim;
    this->depth = 0;
    this->num_nodes = 0;
    this->root = nullptr;
}

bool LineageTree::insert_vector(float *v) {
    printf("adding a new vector %d\n", this->dim);

    if (this->root == nullptr) {
        this->root = new Node(true, this->dim, 4);
    }

    // TODO: search the target node using DFS
    // TODO: alternative approach by using beam search
    std::stack<Node *> dfs_stack;
    dfs_stack.push(this->root);
    Node *target_node = this->root;
    while (!dfs_stack.empty()) {
        Node *cur_node = dfs_stack.top();
        target_node = cur_node;
        dfs_stack.pop();

        if (!cur_node->is_leaf) {
            for (Node* c: cur_node->children) {
                dfs_stack.push(c);
            }
        }
    }

    target_node->insert_vector(v);
    if (target_node->is_full()) {
        // TODO: rebalance tree by pushing up
        printf("handle node full here!\n");
    }

    return true;
}