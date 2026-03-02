#include "elhaylib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DYNAMIC ARRAY
void dynarr_init(dynarr_head* const ptr_head) {
    assert(ptr_head->elem_size > 0 && "Element size must be greater zero.");
    if(ptr_head->dynarr_capacity <= 0) {
        ptr_head->dynarr_capacity = 10;
    }

    if(ptr_head->growth_fac <= 1.0f) {
        ptr_head->growth_fac = 2.0f;
    }

    ptr_head->dynarr_size = 0;
    ptr_head->ptr_first_elem =
        calloc(ptr_head->dynarr_capacity, ptr_head->elem_size);

    if(!ptr_head->ptr_first_elem) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
}

char* dynarr_append(dynarr_head* const ptr_head, const void* element) {
    if(ptr_head->dynarr_size == ptr_head->dynarr_capacity) {
        dynarr_expand(ptr_head);
    }

    void* dest = ptr_head->ptr_first_elem +
                 (ptr_head->dynarr_size * ptr_head->elem_size);
    memcpy(dest, element, ptr_head->elem_size);

    ++ptr_head->dynarr_size;

    return ptr_head->ptr_first_elem;
}

void dynarr_free(dynarr_head* const ptr_head) {
    free(ptr_head->ptr_first_elem);
    ptr_head->dynarr_capacity = 0;
    ptr_head->dynarr_size = 0;
    ptr_head->ptr_first_elem = NULL;
}

char* dynarr_insert(dynarr_head* const ptr_head,
                    void const* element,
                    size_t insert_index) {
    assert(insert_index <= ptr_head->dynarr_size &&
           "Inserts must be within current Dyn-Array bounds.");

    if(ptr_head->dynarr_size == ptr_head->dynarr_capacity) {
        dynarr_expand(ptr_head);
    }

    void* insert_point =
        ptr_head->ptr_first_elem + insert_index * ptr_head->elem_size;
    void* shift_point =
        ptr_head->ptr_first_elem + (insert_index + 1) * ptr_head->elem_size;
    size_t size_move =
        (ptr_head->dynarr_size - insert_index) * ptr_head->elem_size;
    memmove(shift_point, insert_point, size_move);

    memcpy(insert_point, element, ptr_head->elem_size);

    ++ptr_head->dynarr_size;

    return ptr_head->ptr_first_elem;
}

void dynarr_remove(dynarr_head* const ptr_head, size_t index) {
    dynarr_remove_n(ptr_head, index, 1);
}

void dynarr_remove_n(dynarr_head* const ptr_head,
                     size_t index,
                     size_t n_elements) {
    assert(index < ptr_head->dynarr_size &&
           "Index must be within current Dyn-Array bounds.");
    assert(
        index + n_elements - 1 < ptr_head->dynarr_size &&
        "All elements to be removed must be within current Dyn-Array bounds.");

    void* start_point = ptr_head->ptr_first_elem + index * ptr_head->elem_size;
    void* end_point =
        ptr_head->ptr_first_elem + (index + n_elements) * ptr_head->elem_size;
    size_t size_move =
        (ptr_head->dynarr_size - index - n_elements) * ptr_head->elem_size;

    memmove(start_point, end_point, size_move);

    ptr_head->dynarr_size -= n_elements;
}

char* dynarr_concat(dynarr_head* const ptr_array_dest,
                    dynarr_head* const ptr_array_src) {
    assert(ptr_array_dest->elem_size == ptr_array_src->elem_size &&
           "Element sizes of dyn-arrays to be concatenated must be the same");
    size_t available_space =
        ptr_array_dest->dynarr_capacity - ptr_array_dest->dynarr_size;
    while(available_space < ptr_array_src->dynarr_size) {
        dynarr_expand(ptr_array_dest);
        available_space =
            ptr_array_dest->dynarr_capacity - ptr_array_dest->dynarr_size;
    }

    void* insert_point = ptr_array_dest->ptr_first_elem +
                         ptr_array_dest->dynarr_size * ptr_array_src->elem_size;
    void* copy_point = ptr_array_src->ptr_first_elem;
    size_t copy_size = ptr_array_src->dynarr_size * ptr_array_src->elem_size;
    memcpy(insert_point, copy_point, copy_size);

    ptr_array_dest->dynarr_size += ptr_array_src->dynarr_size;

    dynarr_free(ptr_array_src);

    return (char*)insert_point;
}

// internals
void dynarr_expand(dynarr_head* const ptr_head) {
    size_t new_capacity =
        (size_t)(ptr_head->dynarr_capacity * ptr_head->growth_fac);
    void* new_ptr =
        realloc(ptr_head->ptr_first_elem, new_capacity * ptr_head->elem_size);
    if(!new_ptr) {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    }

    ptr_head->ptr_first_elem = new_ptr;
    ptr_head->dynarr_capacity = new_capacity;
}

// LINKED LIST
void linlst_init(linked_list_head* const ptr_head) {
    ptr_head->list_len = 0;

    char zero = 0;
    list_node* sentinel_node_ptr =
        linlst_prepare_node(NODE_CHAR, sizeof(char), &zero);
    sentinel_node_ptr->previous_node = sentinel_node_ptr;
    sentinel_node_ptr->next_node = sentinel_node_ptr;

    ptr_head->ptr_first_node = sentinel_node_ptr;
    ptr_head->ptr_sentinel_node = sentinel_node_ptr;
}

void linlst_append_node(linked_list_head* const ptr_head,
                        node_type dtype,
                        size_t data_size,
                        void const* data) {
    list_node* new_node_ptr = linlst_prepare_node(dtype, data_size, data);
    list_node* ptr_sentinel = ptr_head->ptr_sentinel_node;
    list_node* old_last_node = ptr_sentinel->previous_node;

    if(ptr_head->ptr_first_node == ptr_sentinel) {
        ptr_head->ptr_first_node = new_node_ptr;
    }

    new_node_ptr->next_node = ptr_sentinel;
    new_node_ptr->previous_node = old_last_node;
    ptr_sentinel->previous_node = new_node_ptr;
    old_last_node->next_node = new_node_ptr;

    ptr_head->list_len++;
}

void linlst_prepend_node(linked_list_head* const ptr_head,
                         node_type dtype,
                         size_t data_size,
                         void const* data) {
    list_node* new_node_ptr = linlst_prepare_node(dtype, data_size, data);
    list_node* ptr_sentinel = ptr_head->ptr_sentinel_node;

    new_node_ptr->next_node = ptr_head->ptr_first_node;
    new_node_ptr->previous_node = ptr_sentinel;

    ptr_head->ptr_first_node->previous_node = new_node_ptr;
    ptr_sentinel->next_node = new_node_ptr;
    ptr_head->ptr_first_node = new_node_ptr;

    ptr_head->list_len++;
}

void linlst_insert_node(linked_list_head* const ptr_head,
                        list_node* const pre_node,
                        node_type dtype,
                        size_t data_size,
                        void const* data) {
    list_node* new_node_ptr = linlst_prepare_node(dtype, data_size, data);

    new_node_ptr->previous_node = pre_node;
    new_node_ptr->next_node = pre_node->next_node;
    pre_node->next_node = new_node_ptr;
    new_node_ptr->next_node->previous_node = new_node_ptr;

    ptr_head->list_len++;
}

list_node* linlst_next_circular(linked_list_head* const ptr_head,
                                list_node* cur_node) {
    cur_node = cur_node->next_node;
    return (cur_node == ptr_head->ptr_sentinel_node) ? cur_node->next_node
                                                     : cur_node;
}

list_node* linlst_prev_circular(linked_list_head* const ptr_head,
                                list_node* cur_node) {
    cur_node = cur_node->previous_node;
    return (cur_node == ptr_head->ptr_sentinel_node) ? cur_node->previous_node
                                                     : cur_node;
}

bool linlst_index_insert_node(linked_list_head* const ptr_head,
                              size_t insert_index,
                              node_type dtype,
                              size_t data_size,
                              void const* data) {
    if(insert_index > ptr_head->list_len) {
        return false;
    }

    linlst_index_insert_clamped(ptr_head, insert_index, dtype, data_size, data);
    return true;
}

void linlst_index_insert_clamped(linked_list_head* const ptr_head,
                                 size_t insert_index,
                                 node_type dtype,
                                 size_t data_size,
                                 void const* data) {
    if(insert_index == 0) {
        linlst_prepend_node(ptr_head, dtype, data_size, data);
        return;
    } else if(insert_index >= ptr_head->list_len) {
        linlst_append_node(ptr_head, dtype, data_size, data);
        return;
    }

    list_node_return found_node_buffer = {0};
    linlst_get_node(ptr_head, &found_node_buffer, insert_index);
    linlst_insert_node(ptr_head,
                       found_node_buffer.found_node_ptr->previous_node, dtype,
                       data_size, data);
}

void linlst_get_node(linked_list_head* const ptr_head,
                     list_node_return* found_node_struct,
                     uint64_t index) {
    if(ptr_head->list_len == 0 || ptr_head->list_len - 1 < index) {
        found_node_struct->found_node_ptr = NULL;
        found_node_struct->node_found = false;
        return;
    }

    found_node_struct->node_found = true;
    if(index == 0 && ptr_head->list_len != 0) {
        found_node_struct->found_node_ptr = ptr_head->ptr_first_node;
        return;
    }
    if(index == ptr_head->list_len - 1) {
        found_node_struct->found_node_ptr =
            ptr_head->ptr_sentinel_node->previous_node;
        return;
    }

    found_node_struct->found_node_ptr = ptr_head->ptr_first_node;
    for(uint64_t i = 1; i <= index; ++i) {
        found_node_struct->found_node_ptr =
            (found_node_struct->found_node_ptr)->next_node;
    }
}

void linlst_delete_node(linked_list_head* const ptr_head,
                        list_node* const node) {
    if(node == ptr_head->ptr_sentinel_node) {
        return;
    }

    if(node == ptr_head->ptr_first_node) {
        ptr_head->ptr_first_node = node->next_node;
    }

    list_node* pre_node = node->previous_node;
    list_node* post_node = node->next_node;

    pre_node->next_node = post_node;
    post_node->previous_node = pre_node;

    free(node);
    if(--ptr_head->list_len == 0) {
        ptr_head->ptr_first_node = ptr_head->ptr_sentinel_node;
    }
}

void linlst_index_delete_node(linked_list_head* const ptr_head,
                              size_t delete_index) {
    list_node_return found_node_buffer = {0};
    linlst_get_node(ptr_head, &found_node_buffer, delete_index);
    if(found_node_buffer.node_found) {
        linlst_delete_node(ptr_head, found_node_buffer.found_node_ptr);
    }
}

void linlst_delete_list(linked_list_head* const ptr_head) {
    list_node* cur_node = ptr_head->ptr_first_node;
    while(cur_node != ptr_head->ptr_sentinel_node) {
        list_node* node_next = cur_node->next_node;
        free(cur_node);
        cur_node = node_next;
    }

    list_node* sentinel_node_ptr = ptr_head->ptr_sentinel_node;
    sentinel_node_ptr->previous_node = sentinel_node_ptr;
    sentinel_node_ptr->next_node = sentinel_node_ptr;
    ptr_head->ptr_first_node = sentinel_node_ptr;
    ptr_head->list_len = 0;
}

// internals
list_node* linlst_prepare_node(node_type dtype,
                               size_t data_size,
                               void const* data) {
    list_node* new_node_ptr = calloc(1, sizeof(list_node) + data_size);
    if(!new_node_ptr) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }

    new_node_ptr->data_size = data_size;
    new_node_ptr->dtype = dtype;
    memcpy(new_node_ptr->data, data, data_size);

    return new_node_ptr;
}

// STACK
stack_head* stack_init(size_t elem_size) {
    assert(elem_size > 0 && "Element size must be greater zero.");

    stack_head* stack = calloc(1, sizeof(*stack));
    if(!stack) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }

    stack->impl_array.elem_size = elem_size;

    dynarr_init(&stack->impl_array);

    return stack;
}

bool stack_pop(stack_head* stack, void* out) {
    bool is_ok = stack_peek(stack, out);
    if(is_ok) {
        --stack->impl_array.dynarr_size;
    }

    return is_ok;
}

bool stack_peek(stack_head* stack, void* out) {
    if(stack->impl_array.dynarr_size == 0) {
        return false;
    }

    size_t data_size = stack->impl_array.elem_size;
    char* ptr_stack_base = stack->impl_array.ptr_first_elem;
    size_t index_top = stack->impl_array.dynarr_size - 1;
    char* ptr_top = ptr_stack_base + data_size * index_top;

    memcpy(out, ptr_top, data_size);

    return true;
}

void stack_push(stack_head* stack, void const* in) {
    dynarr_append(&stack->impl_array, in);
}

void stack_free(stack_head* stack) {
    dynarr_free(&stack->impl_array);
    free(stack);
}

// TREE
void tree_init(tree_head* const ptr_head) {
    ptr_head->tree_size = 0;
    ptr_head->tree_root = NULL;
}

void tree_node_root(tree_op_res* op_res,
                    tree_head* const ptr_head,
                    node_type dtype,
                    size_t data_size,
                    void const* data) {
    tree_node* new_node_ptr = tree_prepare_node(dtype, data_size, data);
    ptr_head->tree_root = new_node_ptr;
    ptr_head->tree_size++;

    op_res->code = OK;
    op_res->node_ptr = new_node_ptr;
}

void tree_node_add(tree_op_res* op_res,
                   tree_head* const ptr_head,
                   tree_node* const ptr_parent,
                   node_type dtype,
                   size_t data_size,
                   void const* data) {
    tree_node* new_node_ptr = tree_prepare_node(dtype, data_size, data);
    new_node_ptr->parent = ptr_parent;

    dynarr_head* child_nodes_head = &ptr_parent->children;
    dynarr_append(child_nodes_head, &new_node_ptr);
    ptr_head->tree_size++;

    op_res->code = OK;
    op_res->node_ptr = new_node_ptr;
}

void tree_node_add_at_index(tree_op_res* op_res,
                            tree_head* const ptr_head,
                            tree_node* const ptr_parent,
                            size_t graft_index,
                            node_type dtype,
                            size_t data_size,
                            void const* data) {
    tree_node* new_node_ptr = tree_prepare_node(dtype, data_size, data);
    new_node_ptr->parent = ptr_parent;

    dynarr_head* child_nodes_head = &ptr_parent->children;
    dynarr_insert(child_nodes_head, &new_node_ptr, graft_index);
    ptr_head->tree_size++;

    op_res->code = OK;
    op_res->node_ptr = new_node_ptr;
}

void tree_node_delete(tree_op_res* op_res,
                      tree_head* const ptr_head,
                      tree_node* ptr_node) {
    tree_node* parent = ptr_node->parent;
    dynarr_head* ptr_siblings = &parent->children;

    if(parent == NULL && ptr_head->tree_root != ptr_node) {
        op_res->code = SUBTREE_UNATTACHED;
        op_res->node_ptr = NULL;
        return;
    } else if(ptr_head->tree_root == ptr_node) {
        op_res->code = NODE_IS_ROOT;
        op_res->node_ptr = NULL;
        return;
    }

    // find index of node in parent children array
    size_t node_index = 0;
    tree_node** test_node_ptr = (tree_node**)ptr_siblings->ptr_first_elem;

    while(*test_node_ptr != ptr_node) {
        assert(node_index < ptr_siblings->dynarr_size &&
               "Tree Corruption: Node not found in parent children array.");
        ++node_index;
        ++test_node_ptr;
    }

    // remove ptr from parent children array
    dynarr_remove(ptr_siblings, node_index);
    --ptr_head->tree_size;

    // add children to siblings
    if(ptr_node->children.dynarr_size != 0) {
        dynarr_head* ptr_children = &ptr_node->children;
        dynarr_concat(ptr_siblings, ptr_children);
    }

    tree_free_node(ptr_node);

    op_res->code = OK;
    op_res->node_ptr = parent;
}

void tree_detach_subtree(tree_op_res* op_res,
                         tree_head* const ptr_head,
                         tree_node* ptr_node) {
    if(ptr_node->parent == NULL && ptr_head->tree_root != ptr_node) {
        op_res->code = SUBTREE_UNATTACHED;
        op_res->node_ptr = NULL;
        return;
    } else if(ptr_node->parent == NULL) {
        tree_detach_root(op_res, ptr_head);
        return;
    }
    tree_node* ptr_parent = ptr_node->parent;

    // find index of node in parent children array
    size_t node_index = 0;
    tree_node** test_node_ptr =
        (tree_node**)ptr_parent->children.ptr_first_elem;

    while(*test_node_ptr != ptr_node) {
        assert(node_index < ptr_parent->children.dynarr_size &&
               "Tree Corruption: Node not found in parent children array.");
        ++node_index;
        ++test_node_ptr;
    }

    // remove ptr from parent children array
    dynarr_remove(&ptr_parent->children, node_index);
    ptr_node->parent = NULL;
    size_t subtree_size = tree_count_nodes(ptr_node);
    ptr_head->tree_size -= subtree_size;

    op_res->code = OK;
    op_res->node_ptr = ptr_node;
}

void tree_detach_root(tree_op_res* op_res, tree_head* const ptr_head) {
    op_res->node_ptr = ptr_head->tree_root;
    ptr_head->tree_root = NULL;
    ptr_head->tree_size = 0;
    op_res->code = OK;
}

void tree_graft_subtree(tree_op_res* op_res,
                        tree_head* const ptr_head,
                        tree_node* ptr_new_parent,
                        tree_node* ptr_node,
                        size_t graft_index) {
    if(ptr_node->parent != NULL) {
        dynarr_head children_array = ptr_node->parent->children;
        tree_node** test_node_ptr = (tree_node**)children_array.ptr_first_elem;
        for(size_t i = 0; i < children_array.dynarr_size; ++i) {
            if(*test_node_ptr++ == ptr_node) {
                op_res->code = SUBTREE_ATTACHED;
                op_res->node_ptr = NULL;
                return;
            }
        }
    }

    size_t subtree_size = tree_count_nodes(ptr_node);

    if(ptr_new_parent == NULL) {
        ptr_head->tree_root = ptr_node;
    } else {
        dynarr_head* child_nodes_head = &ptr_new_parent->children;
        dynarr_insert(child_nodes_head, &ptr_node, graft_index);
        ptr_node->parent = ptr_new_parent;
    }
    ptr_head->tree_size += subtree_size;

    op_res->code = OK;
    op_res->node_ptr = ptr_node;
}

void tree_graft_root(tree_op_res* op_res,
                     tree_head* const ptr_head,
                     tree_node* ptr_node) {
    tree_graft_subtree(op_res, ptr_head, NULL, ptr_node, 0);
}

void tree_detach_graft_subtree(tree_op_res* op_res,
                               tree_head* const ptr_src_tree,
                               tree_head* const ptr_dest_tree,
                               tree_node* ptr_new_parent,
                               tree_node* ptr_node,
                               size_t graft_index) {
    tree_detach_subtree(op_res, ptr_src_tree, ptr_node);
    if(op_res->code != OK) {
        return;
    }
    tree_graft_subtree(op_res, ptr_dest_tree, ptr_new_parent, ptr_node,
                       graft_index);
}

tree_node* tree_get_ith_node_ptr(tree_node* ptr_node, size_t i) {
    tree_node** pointer_to_children_pointers =
        (tree_node**)ptr_node->children.ptr_first_elem;
    tree_node* child_node_ptr = *(pointer_to_children_pointers + i);

    return child_node_ptr;
}

size_t tree_count_nodes(tree_node* ptr_node) {
    size_t tree_count = 1;
    size_t children_count = ptr_node->children.dynarr_size;
    if(children_count == 0) {
        return tree_count;
    } else {
        for(size_t i = 0; i < children_count; ++i) {
            tree_node* current_child_ptr = tree_get_ith_node_ptr(ptr_node, i);
            tree_count += tree_count_nodes(current_child_ptr++);
        }
    }

    return tree_count;
}

// To prune a subtree and free its memory it is necessary to
// visit every node and free:
//  1. The node's linked list of pointers and then
//  2. The node itself
// The nodes need to be visited in Post-order(LRN)
//
// Implementation approach:
// 1. Create a stack of pointers to all to be freed nodes in Post-order(LRN)
// 2. Pop the stack and free the node's list and then the node
// void tree_prune(tree_head* const ptr_head, tree_node* ptr_node) {}

// A tree free is a tree prune with the tree root as the prune
// subtree root
// void tree_free(tree_head* const ptr_head) {}

// internals
tree_node* tree_prepare_node(node_type dtype,
                             size_t data_size,
                             void const* data) {
    tree_node* new_node_ptr = calloc(1, sizeof(tree_node) + data_size);
    if(!new_node_ptr) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }

    new_node_ptr->data_size = data_size;
    new_node_ptr->dtype = dtype;
    memcpy(new_node_ptr->data, data, data_size);

    new_node_ptr->children.dynarr_size = 2;
    new_node_ptr->children.elem_size = sizeof(tree_node*);
    dynarr_init(&new_node_ptr->children);

    return new_node_ptr;
}

void tree_free_node(tree_node* const ptr_node) {
    dynarr_free(&ptr_node->children);
    free(ptr_node);
}
