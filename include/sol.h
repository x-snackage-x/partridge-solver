#pragma once

#include <elhaylib.h>
#include <puz.h>

typedef enum GAP_TYPE { VERTICAL, HORIZONTAL, NOT_FOUND } GAP_TYPE;
typedef struct gap_search_result {
    int x_index;
    int y_index;
    int gap;
    GAP_TYPE type;
} gap_search_result;

typedef struct point {
    int x_index;
    int y_index;
} point;

// the bool valid_tiles[] array describes all the tiles
// that have been attempted as children
// if it is exhausted then the tree descent has to move
// to the parent of the selected node
typedef struct node_placement {
    uint8_t tile_type;
    uint8_t x_pos;
    uint8_t y_pos;
    uint16_t valid_tiles;
} node_placement;

typedef bool (*VIS_I_PTR)(bool);
typedef bool (*VIS_C_PTR)(bool, int);
typedef void (*VIS_F_PTR)(int);
typedef void (*VIS_SET_F_PTR)(int, int, int);
typedef void (*VIS_SET_C_PTR)(int, int);

void setup(puzzle_def* start_puzzle);
void set_visualizer(VIS_I_PTR grid_init_func_in,
                    VIS_SET_C_PTR grid_prep_func_in,
                    VIS_F_PTR grid_render_func_in,
                    VIS_F_PTR grid_reset_func_in,
                    VIS_F_PTR grid_record_func_in,
                    VIS_SET_F_PTR block_set_func_in,
                    VIS_SET_F_PTR block_remove_func_in,
                    VIS_C_PTR grid_clean_func_in);
void init_visualizer();
bool solution_search();
