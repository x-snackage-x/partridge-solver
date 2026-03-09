#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <puz.h>
#include <sol.h>
#include <vis.h>

extern FILE* log_fptr;
extern bool print_full_log;

bool visualizer_set;

VIS_I_PTR grid_init_func;
VIS_SET_C_PTR grid_prep_func;
VIS_F_PTR grid_render_func;
VIS_F_PTR grid_reset_func;
VIS_F_PTR grid_record_func;
VIS_SET_F_PTR block_set_func;
VIS_SET_F_PTR block_remove_func;
VIS_C_PTR grid_clean_func;

puzzle_def* my_puzzle;

tree_head placement_record;
tree_op_res tree_result;
size_t node_size;
tree_node* last_placement;

bool is_solvable;
bool is_solved;
uint64_t loop_n;
uint64_t tree_max;

typedef enum my_node_types { NODE_PARTRIDGE = 1001 } my_node_types;

int random_tile_select(uint16_t filter, int max_tile_size);
int largest_tile_select(uint16_t filter, int max_tile_size);

bool line_scan_hor(puzzle_def* puzzle, point* result);
bool find_smallest_gap(puzzle_def* puzzle, gap_search_result* res_struct);
bool is_solvable_gap_cond(puzzle_def* puzzle);

uint16_t set_exhausted_tiles(uint16_t valid_tiles);
int n_ok_tile_types(uint16_t valid_tiles);

void setup(puzzle_def* start_puzzle) {
    my_puzzle = start_puzzle;

    srand(time(NULL));

    // place Dummy root node
    uint16_t valid_tiles = 0xFFFE;
    if(my_puzzle->size == 1) {
        valid_tiles = 0xFFFF;
    }

    node_size = sizeof(node_placement);
    tree_init(&placement_record);

    node_placement* node_buffer = malloc(node_size);
    node_buffer->tile_type = 0;
    node_buffer->x_pos = -1;
    node_buffer->y_pos = -1;
    node_buffer->valid_tiles = valid_tiles;

    tree_node_root(&tree_result, &placement_record, NODE_PARTRIDGE, node_size,
                   node_buffer);
    last_placement = tree_result.node_ptr;
    free(node_buffer);
}

void set_visualizer(VIS_I_PTR grid_init_func_in,
                    VIS_SET_C_PTR grid_prep_func_in,
                    VIS_F_PTR grid_render_func_in,
                    VIS_F_PTR grid_reset_func_in,
                    VIS_F_PTR grid_record_func_in,
                    VIS_SET_F_PTR block_set_func_in,
                    VIS_SET_F_PTR block_remove_func_in,
                    VIS_C_PTR grid_clean_func_in) {
    grid_init_func = grid_init_func_in;
    grid_prep_func = grid_prep_func_in;
    grid_render_func = grid_render_func_in;
    grid_reset_func = grid_reset_func_in;
    grid_record_func = grid_record_func_in;
    block_set_func = block_set_func_in;
    block_remove_func = block_remove_func_in;
    grid_clean_func = grid_clean_func_in;
}

void init_visualizer() {
    visualizer_set = grid_init_func(true);

    if(visualizer_set) {
        grid_prep_func(my_puzzle->grid_dimension, my_puzzle->size);

        // record root tile
        grid_render_func(my_puzzle->grid_dimension);
        grid_reset_func(my_puzzle->grid_dimension);
    }
}

tree_node* record_placement(int selected_tile,
                            int x_pos,
                            int y_pos,
                            tree_node* prev_placement) {
    node_placement* node_buffer = malloc(node_size);
    node_buffer->tile_type = selected_tile;
    node_buffer->x_pos = x_pos;
    node_buffer->y_pos = y_pos;
    node_buffer->valid_tiles = 0xFFFF;

    tree_node_add(&tree_result, &placement_record, prev_placement,
                  NODE_PARTRIDGE, node_size, node_buffer);
    free(node_buffer);
    ++tree_max;

    // visualize placement
    if(visualizer_set) {
        block_set_func(selected_tile, x_pos, y_pos);
        grid_render_func(my_puzzle->grid_dimension);
        grid_reset_func(my_puzzle->grid_dimension);
    }

    return tree_result.node_ptr;
}

uint16_t record_removal(int selected_tile,
                        int x_pos,
                        int y_pos,
                        tree_node* current,
                        tree_node* parent) {
    node_placement* parent_placement_data = (node_placement*)parent->data;

    parent_placement_data->valid_tiles &= ~(1 << (selected_tile - 1));

    // delete node from tree
    tree_node_delete(&tree_result, &placement_record, current);

    // visualize removal
    if(visualizer_set) {
        block_remove_func(selected_tile, x_pos, y_pos);
        grid_render_func(my_puzzle->grid_dimension);
        grid_reset_func(my_puzzle->grid_dimension);
    }

    return parent_placement_data->valid_tiles;
}

bool solution_search() {
    int puzzle_type = my_puzzle->size;

    is_solvable = is_solvable_gap_cond(my_puzzle);
    is_solved = is_puzzle_solved(my_puzzle);

    uint16_t valid_tiles_buffer = 0xFFFF;

    point result_buffer = {0};
    loop_n = 0;
    tree_max = 0;
    while(!is_solved) {
        if(++loop_n % 100000 == 0 && !visualizer_set) {
            printf("Current iter.: %'" PRIu64 " - Tree Size: %'zu Nodes",
                   loop_n, placement_record.tree_size);
            fflush(stdout);
            printf("\r");
        }

        line_scan_hor(my_puzzle, &result_buffer);

        node_placement* placement_data = (node_placement*)last_placement->data;
        placement_data->valid_tiles =
            set_exhausted_tiles(placement_data->valid_tiles);

        // select one tile and place
        RETURN_CODES placement_code = -1;
        do {
            int selected_tile =
                random_tile_select(placement_data->valid_tiles, puzzle_type);
            if(print_full_log)
                fprintf(log_fptr, "Current tile: %d", selected_tile);

            placement_code =
                place_block(my_puzzle, selected_tile, result_buffer.x_index,
                            result_buffer.y_index);

            if(placement_code == SUCCESS) {
                if(print_full_log)
                    fprintf(log_fptr, " - Placement success: true\n");

                last_placement =
                    record_placement(selected_tile, result_buffer.x_index,
                                     result_buffer.y_index, last_placement);
            } else {
                placement_data->valid_tiles &= ~(1 << (selected_tile - 1));

                if(print_full_log)
                    fprintf(log_fptr, " - Placement success: false\n");
            }

            valid_tiles_buffer = placement_data->valid_tiles;
        } while(placement_code != SUCCESS &&
                n_ok_tile_types(valid_tiles_buffer) > 0);

        is_solvable = is_solvable_gap_cond(my_puzzle);
        if(!is_solvable) {
            tree_node* parent = last_placement->parent;
            node_placement cur_placement_data =
                *(node_placement*)last_placement->data;

            remove_block(my_puzzle, cur_placement_data.tile_type,
                         cur_placement_data.x_pos, cur_placement_data.y_pos);

            valid_tiles_buffer = record_removal(
                cur_placement_data.tile_type, cur_placement_data.x_pos,
                cur_placement_data.y_pos, last_placement, parent);

            if(print_full_log)
                fprintf(log_fptr, " Remove tile: %d, Pos. (%2d,%2d)\n",
                        cur_placement_data.tile_type, cur_placement_data.x_pos,
                        cur_placement_data.y_pos);

            last_placement = parent;
        }

        while(n_ok_tile_types(valid_tiles_buffer) == 0) {
            tree_node* parent = last_placement->parent;
            node_placement cur_placement_data =
                *(node_placement*)last_placement->data;

            if(last_placement == placement_record.tree_root) {
                if(print_full_log)
                    fprintf(log_fptr,
                            " Remove tile: %d, Pos. (%2d,%2d) - Root\n\n",
                            cur_placement_data.tile_type,
                            cur_placement_data.x_pos, cur_placement_data.y_pos);

                if(visualizer_set) {
                    block_remove_func(cur_placement_data.tile_type,
                                      cur_placement_data.x_pos,
                                      cur_placement_data.y_pos);
                    grid_render_func(my_puzzle->grid_dimension);
                    grid_reset_func(my_puzzle->grid_dimension);
                }

                goto finish;
            }

            remove_block(my_puzzle, cur_placement_data.tile_type,
                         cur_placement_data.x_pos, cur_placement_data.y_pos);

            valid_tiles_buffer = record_removal(
                cur_placement_data.tile_type, cur_placement_data.x_pos,
                cur_placement_data.y_pos, last_placement, parent);

            if(print_full_log)
                fprintf(log_fptr, " Remove tile: %d, Pos. (%2d,%2d)\n",
                        cur_placement_data.tile_type, cur_placement_data.x_pos,
                        cur_placement_data.y_pos);

            last_placement = parent;
        }

        is_solved = is_puzzle_solved(my_puzzle);
    }

finish:
    return is_puzzle_solved(my_puzzle);
}

bool line_scan_hor(puzzle_def* puzzle, point* result) {
    int** grid = puzzle->puzzle_grid;
    for(int i = 0; i < puzzle->grid_dimension; ++i) {
        for(int j = 0; j < puzzle->grid_dimension; ++j) {
            if(grid[i][j] == 0) {
                result->x_index = j;
                result->y_index = i;
                return true;
            }
        }
    }

    return false;
}

// - Find smallest, bounded gap in all line scans
//     * Horizontally + Vertically
bool find_smallest_gap(puzzle_def* puzzle, gap_search_result* res_struct) {
    res_struct->x_index = 0;
    res_struct->y_index = 0;
    res_struct->gap = 0;
    res_struct->type = NOT_FOUND;

    if(is_puzzle_solved(puzzle)) {
        return false;
    }

    int** grid = puzzle->puzzle_grid;

    // Horizontal Scan
    int x_index_hor_scan = 0;
    int y_index_hor_scan = 0;
    int size_hor_scan = puzzle->grid_dimension + 1;
    for(int i = 0; i < puzzle->grid_dimension; ++i) {
        int line_gap_count = 0;
        int index_buffer = 0;
        bool am_counting = false;
        for(int j = 0; j < puzzle->grid_dimension; ++j) {
            if(grid[i][j] == 0 && am_counting) {
                ++line_gap_count;
            } else if(grid[i][j] == 0 && !am_counting) {
                ++line_gap_count;
                index_buffer = j;
                am_counting = true;
            } else if((grid[i][j] != 0 ||
                       // Handle grid edge
                       j == puzzle->grid_dimension - 1) &&
                      am_counting) {
                if(size_hor_scan > line_gap_count) {
                    size_hor_scan = line_gap_count;
                    x_index_hor_scan = i;
                    y_index_hor_scan = index_buffer;
                }
                index_buffer = 0;
                am_counting = false;
            }
        }
        if(size_hor_scan > line_gap_count && am_counting) {
            size_hor_scan = line_gap_count;
            x_index_hor_scan = i;
            y_index_hor_scan = index_buffer;
        }
    }

    // Vertical Scan
    int x_index_ver_scan = 0;
    int y_index_ver_scan = 0;
    int size_ver_scan = puzzle->grid_dimension + 1;
    for(int j = 0; j < puzzle->grid_dimension; ++j) {
        int column_gap_count = 0;
        int index_buffer = 0;
        bool am_counting = false;
        for(int i = 0; i < puzzle->grid_dimension; ++i) {
            if(grid[i][j] == 0 && am_counting) {
                ++column_gap_count;
            } else if(grid[i][j] == 0 && !am_counting) {
                ++column_gap_count;
                index_buffer = i;
                am_counting = true;
            } else if((grid[i][j] != 0 ||
                       // Handle grid edge
                       j == puzzle->grid_dimension - 1) &&
                      am_counting) {
                if(size_ver_scan > column_gap_count) {
                    size_ver_scan = column_gap_count;
                    x_index_ver_scan = index_buffer;
                    y_index_ver_scan = j;
                }
                index_buffer = 0;
                am_counting = false;
            }
        }
        if(size_ver_scan > column_gap_count && am_counting) {
            size_ver_scan = column_gap_count;
            x_index_ver_scan = index_buffer;
            y_index_ver_scan = j;
        }
    }

    if(size_hor_scan <= size_ver_scan) {
        res_struct->x_index = x_index_hor_scan;
        res_struct->y_index = y_index_hor_scan;
        res_struct->gap = size_hor_scan;
        res_struct->type = HORIZONTAL;
    } else {
        res_struct->x_index = x_index_ver_scan;
        res_struct->y_index = y_index_ver_scan;
        res_struct->gap = size_ver_scan;
        res_struct->type = VERTICAL;
    }

    return true;
}

// Checks if puzzle is in solvable state by:
// - Finding smallest, bounded gap in all line scans
//     * Horizontally and Vertically
// - If smaller than available smallest piece -> not solvable
bool is_solvable_gap_cond(puzzle_def* puzzle) {
    gap_search_result result;
    bool gap_bool = find_smallest_gap(puzzle, &result);

    int smallest_available_tile = 0;
    for(int i = puzzle->size; i > 0; --i) {
        int n_tiles_i = get_n_available_pieces(puzzle, i);
        if(n_tiles_i > 0)
            smallest_available_tile = i;
    }

    bool is_solvable = true;
    if(gap_bool && result.gap <= puzzle->size) {
        if(result.gap < smallest_available_tile)
            is_solvable = false;
    }

    return is_solvable;
}

int random_tile_select(uint16_t filter, int max_tile_size) {
    int* candidate_tiles = calloc((size_t)max_tile_size, sizeof(int));
    int j = 0;
    for(int i = 1; i <= max_tile_size; ++i) {
        if((filter >> (i - 1)) & 1) {
            candidate_tiles[j++] = i;
        }
    }

    bool tile_not_found = true;
    int random_tile = 0;
    do {
        random_tile = rand() % max_tile_size + 1;
        for(int i = 0; i < j && tile_not_found; ++i) {
            if(candidate_tiles[i] == random_tile) {
                tile_not_found = false;
            }
        }
    } while(tile_not_found);
    free(candidate_tiles);

    return random_tile;
}

int largest_tile_select(uint16_t filter, int max_tile_size) {
    int selected_tile = 0;
    for(int i = max_tile_size; i > 0; --i) {
        if((filter >> i) & 1) {
            selected_tile = i;
            break;
        }
    }

    return selected_tile;
}

uint16_t set_exhausted_tiles(uint16_t valid_tiles) {
    for(int i = 1; i <= my_puzzle->size; ++i) {
        int n_tile_pcs = get_n_available_pieces(my_puzzle, i);
        if(n_tile_pcs == 0) {
            valid_tiles &= ~(1 << (i - 1));
        }
    }
    return valid_tiles;
}

int n_ok_tile_types(uint16_t valid_tiles) {
    int available_tiles = 0;
    for(int i = 0; i < my_puzzle->size; ++i) {
        available_tiles += (valid_tiles >> i) & 1;
    }
    return available_tiles;
}
