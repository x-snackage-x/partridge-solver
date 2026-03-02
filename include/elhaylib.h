#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#pragma warning(disable : 4200)
#endif

typedef enum ERROR_CODES {
    OK,
    SUBTREE_UNATTACHED,
    SUBTREE_ATTACHED,
    NODE_IS_ROOT
} ERROR_CODES;

// DYNAMIC ARRAY
typedef struct dynarr_head {
    size_t elem_size;
    size_t dynarr_size;
    size_t dynarr_capacity;
    float growth_fac;
    char* ptr_first_elem;
} dynarr_head;

void dynarr_init(dynarr_head* const ptr_head);
char* dynarr_append(dynarr_head* const ptr_head, void const* element);
void dynarr_free(dynarr_head* const ptr_head);

char* dynarr_insert(dynarr_head* const ptr_head,
                    void const* element,
                    size_t insert_index);
void dynarr_remove(dynarr_head* const ptr_head, size_t index);
void dynarr_remove_n(dynarr_head* const ptr_head,
                     size_t index,
                     size_t n_elements);

char* dynarr_concat(dynarr_head* const ptr_array_dest,
                    dynarr_head* const ptr_array_src);

// internals
void dynarr_expand(dynarr_head* const ptr_head);

// LINKED LIST
typedef enum node_type {
    // Standard primitive types
    NODE_BOOL = 1,
    NODE_CHAR,
    NODE_UCHAR,
    NODE_SHORT,
    NODE_USHORT,
    NODE_INT,
    NODE_UINT,
    NODE_LONG,
    NODE_ULONG,
    NODE_LONGLONG,
    NODE_ULONGLONG,
    NODE_FLOAT,
    NODE_DOUBLE,
    NODE_LONGDOUBLE,

    // Fixed-width integer types
    NODE_INT8,
    NODE_UINT8,
    NODE_INT16,
    NODE_UINT16,
    NODE_INT32,
    NODE_UINT32,
    NODE_INT64,
    NODE_UINT64,

    // Reserved range for user-defined types
    NODE_USER_START = 1000
} node_type;

typedef struct list_node list_node;
typedef struct linked_list_head {
    size_t list_len;
    list_node* ptr_first_node;
    list_node* ptr_sentinel_node;
} linked_list_head;

struct list_node {
    list_node* next_node;
    list_node* previous_node;
    node_type dtype;
    size_t data_size;
    char data[];
};

typedef struct list_node_return {
    bool node_found;
    list_node* found_node_ptr;
} list_node_return;

void linlst_init(linked_list_head* const ptr_head);
void linlst_append_node(linked_list_head* const ptr_head,
                        node_type dtype,
                        size_t data_size,
                        void const* data);
void linlst_prepend_node(linked_list_head* const ptr_head,
                         node_type dtype,
                         size_t data_size,
                         void const* data);
void linlst_insert_node(linked_list_head* const ptr_head,
                        list_node* const pre_node,
                        node_type dtype,
                        size_t data_size,
                        void const* data);
list_node* linlst_next_circular(linked_list_head* const ptr_head,
                                list_node* cur_node);
list_node* linlst_prev_circular(linked_list_head* const ptr_head,
                                list_node* cur_node);
bool linlst_index_insert_node(linked_list_head* const ptr_head,
                              size_t insert_index,
                              node_type dtype,
                              size_t data_size,
                              void const* data);
void linlst_index_insert_clamped(linked_list_head* const ptr_head,
                                 size_t insert_index,
                                 node_type dtype,
                                 size_t data_size,
                                 void const* data);
void linlst_get_node(linked_list_head* const ptr_head,
                     list_node_return* found_node_struct,
                     uint64_t index);
void linlst_delete_node(linked_list_head* const ptr_head,
                        list_node* const node);
void linlst_index_delete_node(linked_list_head* const ptr_head,
                              size_t delete_index);
void linlst_delete_list(linked_list_head* const ptr_head);
// internals
list_node* linlst_prepare_node(node_type dtype,
                               size_t data_size,
                               void const* data);

// STACK
typedef struct stack_head {
    dynarr_head impl_array;
} stack_head;

stack_head* stack_init(size_t elem_size);
bool stack_pop(stack_head* stack, void* out);
bool stack_peek(stack_head* stack, void* out);
void stack_push(stack_head* stack, void const* in);
void stack_free(stack_head* stack);

// TREE
typedef struct tree_node tree_node;
typedef struct tree_head {
    size_t tree_size;
    tree_node* tree_root;
} tree_head;

struct tree_node {
    tree_node* parent;
    dynarr_head children;
    node_type dtype;
    size_t data_size;
    char data[];
};

typedef struct tree_op_res {
    ERROR_CODES code;
    tree_node* node_ptr;
} tree_op_res;

void tree_init(tree_head* const ptr_head);
void tree_node_root(tree_op_res* op_res,
                    tree_head* const ptr_head,
                    node_type dtype,
                    size_t data_size,
                    void const* data);
void tree_node_add(tree_op_res* op_res,
                   tree_head* const ptr_head,
                   tree_node* const ptr_parent,
                   node_type dtype,
                   size_t data_size,
                   void const* data);
void tree_node_add_at_index(tree_op_res* op_res,
                            tree_head* const ptr_head,
                            tree_node* const ptr_parent,
                            size_t graft_index,
                            node_type dtype,
                            size_t data_size,
                            void const* data);

void tree_node_delete(tree_op_res* op_res,
                      tree_head* const ptr_head,
                      tree_node* ptr_node);

void tree_detach_subtree(tree_op_res* op_res,
                         tree_head* const ptr_head,
                         tree_node* ptr_node);
void tree_detach_root(tree_op_res* op_res, tree_head* const ptr_head);
void tree_graft_subtree(tree_op_res* op_res,
                        tree_head* const ptr_head,
                        tree_node* ptr_new_parent,
                        tree_node* ptr_node,
                        size_t graft_index);
void tree_graft_root(tree_op_res* op_res,
                     tree_head* const ptr_head,
                     tree_node* ptr_node);
void tree_detach_graft_subtree(tree_op_res* op_res,
                               tree_head* const ptr_src_tree,
                               tree_head* const ptr_dest_tree,
                               tree_node* ptr_new_parent,
                               tree_node* ptr_node,
                               size_t graft_index);

// Free the subtree nodes in Post-Order traversal
size_t tree_prune(tree_op_res* op_res,
                  tree_head* const ptr_head,
                  tree_node* ptr_node);
void tree_free(tree_op_res* op_res, tree_head* const ptr_head);

tree_node* tree_get_ith_node_ptr(tree_node* ptr_node, size_t i);
size_t tree_count_nodes(tree_node* ptr_node);

// TREE Traversals
// Approach: a subtree node ptr is passed with a initialized stack
// The stack is filled with the pointers to the tree nodes in the
// desired order.
// If the tree node ptr is NULL then the tree root will be taken as
// the subtree node

void tree_traversal_level_order(stack_head* stack,
                                tree_head* const ptr_head,
                                tree_node* ptr_node);

void tree_traversal_pre_order(stack_head* stack,
                              tree_head* const ptr_head,
                              tree_node* ptr_node);
void tree_traversal_post_order(stack_head* stack,
                               tree_head* const ptr_head,
                               tree_node* ptr_node);

// in_order_partition defines the the partition for in-order traversal
// 0.0f: subtree is all R -> NR
// 1.0f: subtree is all L -> LN
// default is 0.5f
// -> expected behaviour for binary trees
// Note: User is expected to set value
void tree_traversal_in_order(float in_order_partition,
                             stack_head* stack,
                             tree_head* const ptr_head,
                             tree_node* ptr_node);

// internals
tree_node* tree_prepare_node(node_type dtype,
                             size_t data_size,
                             void const* data);
void tree_free_node(tree_node* const ptr_node);