#include "lineagetree/lineagetree.h"
#include "lineagetree/node.h"
#include "lineagetree/utils.h"
#include <stack>
#include <queue>

const uint32_t NODE_MAX_CENTROID = 13;

LineageTree::LineageTree(uint32_t dim) {
    this->dim = dim;
    this->root = nullptr;
    this->next_vid = 0;
}

bool LineageTree::insert_vector(float *v) {
    // printf("adding a new vector %d\n", this->dim);

    // handle if this tree is still empty, i.e. the root is null
    if (this->root == nullptr) {
        this->root = new Node(true,
                              this->dim,
                              NODE_MAX_CENTROID);
    }

    Node *target_node = this->find_target_insert_node(v);
    assert(target_node != nullptr && target_node->is_leaf == true);
    target_node->insert_vector(this->next_vid, v);
    if (target_node->is_full()) {
        // split the full node into two nodes, potentially creating
        // a new parent node (pushing up)
        this->split_node(target_node);
    }

    this->next_vid++;

    return true;
}

Node *LineageTree::find_target_insert_node(float *v) const {
    Node *target_node = this->root;

    // TODO: search the target node using best first search
    // TODO: alternative approach by using beam search
    std::stack<Node *> candidate_stack;
    candidate_stack.push(this->root);

    // candidate_node;

    // Lineage Tree -> Node -> Cluster
    //                      -> Node

    while (!candidate_stack.empty()) {
        Node *cur_node = candidate_stack.top();
        target_node = cur_node;
        candidate_stack.pop();

        if (cur_node->is_leaf) {
            continue;
        }

        // find near centroids
        // TODO: can we use the pre-computed distance (PCD) here?
        typedef std::pair<float, uint32_t> dist_pair;
        std::priority_queue<
                dist_pair,
                std::vector<dist_pair>,
                std::greater<> > pq;
        for (int i = 0; i < cur_node->centroids.size(); ++i) {
            float tmp_dist = l2_sq_distance(this->dim,
                                            v,
                                            cur_node->centroids[i]);
            pq.emplace(tmp_dist, i);
        }

        // push the closest centroid into the candidate list
        candidate_stack.push(cur_node->children[pq.top().second]);
        float min_dist = pq.top().first;
        pq.pop();

        // handle if the vector v has roughly equal distance to multiple
        // centroids. For instance: c1 ----  v ---- c2; v is roughly in
        // between c1 and c2, so we put both c1 and c2 into the candidate list.
        // Here, we consider if the difference between v-c1 and v-c2
        // is within 10%.
        // TODO: candidate_stack should be a candidate heap instead (?)
        while (!pq.empty()) {
            if (pq.top().first <= 1.1 * min_dist) {
                candidate_stack.push(cur_node->children[pq.top().second]);
            } else {
                break;
            }
            pq.pop();
        }

    }

    return target_node;
}

std::vector<float *> LineageTree::approximate_search(uint32_t k) const {
    return std::vector<float *>();
}

std::vector<uint32_t> LineageTree::approximate_search2(uint32_t k, float *q) const {
    // TODO: search the target node using best first search
    // TODO: alternative approach by using beam search
    std::stack<Node *> candidate_stack;
    typedef std::pair<float, uint32_t> dist_pair;
    std::priority_queue<
            dist_pair,
            std::vector<dist_pair>,
            std::greater<> > candidate_result;
    candidate_stack.push(this->root);

    while (!candidate_stack.empty()) {
        Node *cur_node = candidate_stack.top();
        candidate_stack.pop();

        if (cur_node->is_leaf) {
            // TODO: only evaluate in the closest centroid
            for (int i = 0; i < cur_node->centroids.size(); ++i) {
                for (int j = 0; j < cur_node->clusters[i]->vectors.size(); ++j) {
                    float *cur_v = cur_node->clusters[i]->vectors[j];
                    uint32_t cur_vid = cur_node->clusters[i]->vid[j];
                    float tmp_dist = l2_sq_distance(this->dim,
                                                    q,
                                                    cur_v);
                    candidate_result.emplace(tmp_dist, cur_vid);
                    if (candidate_result.size() > k) {
                        // TODO: purge candidate result queue
                    }
                }
            }
            continue;
        }

        // find the nearest centroids
        // TODO: can we use the pre-computed distance (PCD) here?
        std::priority_queue<
                dist_pair,
                std::vector<dist_pair>,
                std::greater<> > pq;
        for (int i = 0; i < cur_node->centroids.size(); ++i) {
            float tmp_dist = l2_sq_distance(this->dim,
                                            q,
                                            cur_node->centroids[i]);
            pq.emplace(tmp_dist, i);
        }

        // push the closest centroid into the candidate list
        candidate_stack.push(cur_node->children[pq.top().second]);
        float min_dist = pq.top().first;
        pq.pop();

        // handle if the vector v has roughly equal distance to multiple
        // centroids. For instance: c1 ----  v ---- c2; v is roughly in
        // between c1 and c2, so we put both c1 and c2 into the candidate list.
        // Here, we consider if the difference between v-c1 and v-c2
        // is within 10%.
        // TODO: candidate_stack should be a candidate heap instead (?)
        while (!pq.empty()) {
            if (pq.top().first <= 1.1 * min_dist) {
                candidate_stack.push(cur_node->children[pq.top().second]);
            } else {
                break;
            }
            pq.pop();
        }

    }

    std::vector<uint32_t> result;
    for (int i = 0; i < k; ++i) {
        result.push_back(candidate_result.top().second);
        candidate_result.pop();
    }

    return result;
}


void LineageTree::split_node(Node *n) {
    // printf(">>> splitting node %p\n", n); n->print();

    //     │                     ┌─------┴-------┐
    // ┌───┴────┐         ┌──────┴─────┐   ┌─────┴──────┐
    // │ Node n │   ==>   │   Node n   │   │ right_node │
    // └────────┘         └────────────┘   └────────────┘
    // When splitting a node into two nodes, we keep the current node n and
    // create a new node named right_node. To split a node, we need to split
    // the centroids (and children) in this node into two sets; the first one
    // is for this node n, and another for the newly created right_node.

    // note that the is_leaf in the right_node depends on whether this node n
    // is a leaf_node or not; splitting a non-leaf node generate another
    // non-leaf node.
    Node *parent_node = n->parent;
    Node *right_node = new Node(n->is_leaf, this->dim, n->max_centroid);

    // handle root node, that is the only node whose parent is null
    if (parent_node == nullptr) {
        parent_node = new Node(false, this->dim, n->max_centroid);
        this->root = parent_node;
        n->parent = parent_node;
        n->parent_id = 0;
        right_node->parent = parent_node;
    }

    // populate the newly created right node
    // 1: start with filling the attributes related to the parent
    right_node->parent = parent_node;
    right_node->parent_id = n->parent_id + 1; // TODO: revisit this when the parent is also full

    // 2: split the centroids in this node into two sets of centroids
    //    using k-means clustering, where k=2.
    uint32_t k = 2;
    std::vector<float *> new_sets_ctr;                  // new sets of centroids
    std::vector<int> ctrd_sid(n->centroids.size());  // set id for centroids

    // the default center of the set is the first and the last centroid
    // of this node
    float *new_c1 = (float *) malloc(this->dim * sizeof(float));
    memcpy(new_c1, n->centroids[0], this->dim);
    new_sets_ctr.push_back(new_c1);
    float *new_c2 = (float *) malloc(this->dim * sizeof(float));
    memcpy(new_c2, n->centroids[n->centroids.size() - 1], this->dim);
    new_sets_ctr.push_back(new_c2);

    // 2.1 initial assignment, split the centroids into two set of centroids
    for (int i = 0; i < n->centroids.size(); i++) {
        ctrd_sid[i] = 0;
        if (i > n->centroids.size() / 2) ctrd_sid[i] = 1;
    }

    // 2.2 run the k-means iteration until the assignment is stable, i.e, no
    //     more changes of centroids assignment to the sets.
    uint32_t num_iteration = 0;
    while (true) {
        bool any_changes = false;
        num_iteration++;

        for (int i = 0; i < n->centroids.size(); ++i) {
            float *cur_c = n->centroids[i];

            // for each centroid, find the closest set's center to it, starting
            // from the first set's center
            float min_dist = l2_sq_distance(this->dim,
                                            cur_c,
                                            new_sets_ctr[0]);

            for (int j = 1; j < new_sets_ctr.size(); ++j) {
                float tmp_dist = l2_sq_distance(this->dim,
                                                cur_c,
                                                new_sets_ctr[j]);
                if (tmp_dist < min_dist) {
                    min_dist = tmp_dist;
                    if (ctrd_sid[i] != j) {
                        ctrd_sid[i] = j;
                        any_changes = true;
                    }
                }
            }
        }

        // at the end of each iteration, we recalculate the set's center
        std::vector<int> num_cen_set(k);            // #centroids in each set
        for (auto &new_ctr: new_sets_ctr) {
            for (int d = 0; d < this->dim; ++d) {
                new_ctr[d] = 0.0;                      // reset the set's center
            }
        }

        // recalculate the center for each set of centroids
        for (int i = 0; i < n->centroids.size(); i++) {
            update_centroid(
                    num_cen_set[ctrd_sid[i]],
                    this->dim,
                    new_sets_ctr[ctrd_sid[i]],
                    n->centroids[i]);
            num_cen_set[ctrd_sid[i]]++;
        }

        // stop when there is no changes
        if (!any_changes) {
            break;
        }
    }

    // 3. assigning the centroids based on the previous set assignment (k-means)
    std::vector<float *> updated_centroids;
    std::vector<Node *> updated_children;
    std::vector<Cluster *> updated_clusters;
    uint32_t cur_node_cur_parent_id = 0;
    uint32_t right_node_cur_parent_id = 0;
    for (int i = 0; i < n->centroids.size(); i++) {
        // centroids that stay in this node n
        if (ctrd_sid[i] == 0) {
            updated_centroids.push_back(n->centroids[i]);
            if (n->is_leaf) {
                updated_clusters.push_back(n->clusters[i]);
            } else {
                updated_children.push_back(n->children[i]);
                n->children[i]->parent_id = cur_node_cur_parent_id;
                cur_node_cur_parent_id++;
            }
        }

        // centroids that move to the newly created right_node
        if (ctrd_sid[i] == 1) {
            right_node->centroids.push_back(n->centroids[i]);
            if (right_node->is_leaf) {
                right_node->clusters.push_back(n->clusters[i]);
            } else {
                Node *moved_child = n->children[i];
                right_node->children.push_back(moved_child);
                moved_child->parent = right_node;
                moved_child->parent_id = right_node_cur_parent_id;
                right_node_cur_parent_id++;
            }
        }
    }

    // 3.1 update the centroids, clusters or children of this node n
    n->centroids = updated_centroids;
    if (n->is_leaf) {
        n->clusters = updated_clusters;
    } else {
        n->children = updated_children;
    }

    // 4. populate the centroids in the parent node
    if (parent_node->centroids.empty()) {
        // handle if the parent is empty (we just created it in this split)
        parent_node->centroids.push_back(new_sets_ctr[0]);
        parent_node->children.push_back(n);
        parent_node->centroids.push_back(new_sets_ctr[1]);
        parent_node->children.push_back(right_node);

    } else {
        // handle if the parent already has some centroids previously,
        // we need to insert the centroid of our newly created right_node
        std::vector<float *> parent_new_centroids;
        std::vector<Node *> parent_new_children;
        for (int i = 0; i < parent_node->centroids.size(); ++i) {
            parent_new_centroids.push_back(parent_node->centroids[i]);
            parent_new_children.push_back(parent_node->children[i]);

            // insert the centroid of the newly created node
            if (i == n->parent_id) {
                free(parent_new_centroids[i]);
                parent_new_centroids[i] = new_sets_ctr[0];
                parent_new_centroids.push_back(new_sets_ctr[1]);
                parent_new_children.push_back(right_node);
            }
        }
        parent_node->centroids = parent_new_centroids;
        parent_node->children = parent_new_children;
    }

    // clear the insertion metadata in the node n
    // TODO: revisit whether these metadata are needed
    n->insert_md_old_centroid = nullptr;
    n->insert_md_split_centroid_idx = 0;

    // 5. handle if the parent node is also full
    if (parent_node->is_full()) {
        // printf("tree: whoops, my parent also need splitting!\n");
        split_node(parent_node);
    }

    // printf("parent:\n");
    // parent_node->print();

    // printf("this-node:\n");
    // n->print();

    // printf("right-node:\n");
    // right_node->print();
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

void LineageTree::print_leaf_nodes() const {
    uint32_t num_leaf_node = 0;
    std::stack<Node *> dfs_stack;
    dfs_stack.push(this->root);
    while (!dfs_stack.empty()) {
        Node *cur_node = dfs_stack.top();
        dfs_stack.pop();

        if (cur_node->is_leaf) {
            num_leaf_node++;

            printf("|");
            for (int i = 0; i < cur_node->centroids.size(); ++i) {
                if (i != 0) {
                    printf(",");
                }
                printf("%ld", cur_node->clusters[i]->vectors.size());
            }
            printf("| ");
        }

        if (!cur_node->children.empty()) {
            for (Node *c: cur_node->children) {
                dfs_stack.push(c);
            }
        }
    }
}

