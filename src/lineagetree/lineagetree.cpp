#include "lineagetree/lineagetree.h"
#include "lineagetree/utils.h"
#include <stack>

LineageTree::LineageTree(uint32_t dim) {
    printf("LineageTree constructor\n");

    this->dim = dim;
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
            for (Node *c: cur_node->children) {
                dfs_stack.push(c);
            }
        }
    }

    assert(target_node != nullptr && target_node->is_leaf == true);
    target_node->insert_vector(v);
    if (target_node->is_full()) {
        // split the full node into two nodes, potentially creating
        // a new parent node (pushing up)
        this->split_node(target_node);
    }

    return true;
}

void LineageTree::split_node(Node *n) {
    printf(">>> splitting node %p\n", n);
    n->print();
    Node *parent_node = n->parent;
    Node *right_node = new Node(true, this->dim, n->max_centroid);

    // handle root node, that is when the parent is null
    if (parent_node == nullptr) {
        parent_node = new Node(false, this->dim, n->max_centroid);
        this->root = parent_node;
        n->parent = parent_node;
        n->parent_id = 0;
        right_node->parent = parent_node;
    }

    // populate the right node, 1: starting with the parent metadata
    right_node->parent = parent_node;
    right_node->parent_id = n->parent_id + 1; // TODO: revisit this when the parent is also full

    // 2: move half of the centroids (also the clusters & children) in node n
    // into the newly created right_node
    uint32_t cur_parent_id = 0;
    uint32_t num_moved_centroids = 0;
    for (int i = n->centroids.size() / 2; i < n->centroids.size(); i++) {
        num_moved_centroids++;
        right_node->centroids.push_back(n->centroids[i]);
        if (n->is_leaf) {
            right_node->clusters.push_back(n->clusters[i]);
        } else {
            Node *moved_child = n->children[i];
            right_node->children.push_back(moved_child);
            moved_child->parent_id = cur_parent_id;
            cur_parent_id++;
            moved_child->parent = right_node;
        }
    }
    // remove the half-end of the centroid from the node n
    while (num_moved_centroids > 0) {
        num_moved_centroids--;
        n->centroids.pop_back();
        if (n->is_leaf) {
            n->clusters.pop_back();
        } else {
            n->children.pop_back();
        }
    }

    // populate the centroids in the parent node
    if (parent_node->centroids.empty()) {
        // handle if the parent is empty (we just created it in this split)

        // TODO: use the centroid metadata in node n
        parent_node->centroids.push_back(
                get_centroid(this->dim, n->centroids));
        parent_node->children.push_back(n);
        parent_node->centroids.push_back(
                get_centroid(this->dim, right_node->centroids));
        parent_node->children.push_back(right_node);
    } else {
        // handle if the parent already have some centroids previously
        // we need to insert the centroid of our newly created right_node
        std::vector<float *> parent_new_centroids;
        std::vector<Node *> parent_new_children;
        for (int i = 0; i < parent_node->centroids.size(); ++i) {
            parent_new_centroids.push_back(parent_node->centroids[i]);
            parent_new_children.push_back(parent_node->children[i]);
            // recalculate the centroid of the split node n
            // TODO: use the centroid metadata in node n
            if (i == n->parent_id) {
                free(parent_new_centroids[i]);
                parent_new_centroids[i] = get_centroid(this->dim,
                                                       n->centroids);
                parent_new_centroids.push_back(
                        get_centroid(this->dim,
                                     right_node->centroids));
                parent_new_children.push_back(right_node);
            }
        }
        parent_node->centroids = parent_new_centroids;
        parent_node->children = parent_new_children;
    }

    // clear insertion metadata in the node n
    n->insert_md_old_centroid = nullptr;
    n->insert_md_split_centroid_idx = 0;

    // TODO: handle if the parent node is also full
    if (parent_node->is_full()) {
        printf("tree: whoops, my parent also need splitting!\n");
        split_node(parent_node);
    }

    printf("parent:\n");
    parent_node->print();

    printf("this-node:\n");
    n->print();

    printf("right-node:\n");
    right_node->print();
}

uint32_t LineageTree::get_depth() const {
    uint32_t depth = 0;
    Node *cur_node = this->root;
    while (cur_node != nullptr) {
        depth++;
        if (!cur_node->children.empty()) {
            cur_node = cur_node->children[0];
        } else {
            cur_node = nullptr;
        }
    }
    return depth;
}

uint32_t LineageTree::get_num_nodes() const {
    uint32_t num_node = 0;
    std::stack<Node *> dfs_stack;
    dfs_stack.push(this->root);
    while (!dfs_stack.empty()) {
        Node *cur_node = dfs_stack.top();
        num_node++;
        dfs_stack.pop();
        if (!cur_node->children.empty()) {
            for (Node *c: cur_node->children) {
                dfs_stack.push(c);
            }
        }
    }
    return num_node;
}

uint32_t LineageTree::get_num_leaf_nodes() const {
    uint32_t num_leaf_node = 0;
    std::stack<Node *> dfs_stack;
    dfs_stack.push(this->root);
    while (!dfs_stack.empty()) {
        Node *cur_node = dfs_stack.top();
        dfs_stack.pop();

        if (cur_node->is_leaf) {
            num_leaf_node++;
        }

        if (!cur_node->children.empty()) {
            for (Node *c: cur_node->children) {
                dfs_stack.push(c);
            }
        }
    }
    return num_leaf_node;
}
