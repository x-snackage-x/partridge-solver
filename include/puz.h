#pragma once

#include <elhaylib.h>
#include <stdbool.h>

typedef struct block_def {
    int size;
    int free_pieces;
} block_def;

typedef struct puzzle_def {
    int size;
    int grid_dimension;
    int** puzzle_grid;
    dynarr_head* blocks;
} puzzle_def;

typedef struct puz_entry {
    int type;
    int x_pos;
    int y_pos;
} puz_entry;

typedef enum RETURN_CODES {
    SUCCESS,
    NO_FREE_PIECES,
    CONFLICT_ON_GRID,
    NO_BLOCK_AT_POSITION,
    CONFLICTING_BLOCK_TYPES,
    CORRESPONDING_MOVE_NOT_FOUND
} RETURN_CODES;

// Only size is expected to be set
// size := dimension of the larges tile
void init_puzzle(puzzle_def* puzzle, bool override);
void free_puzzle(puzzle_def* puzzle);

RETURN_CODES place_block(puzzle_def* puzzle,
                         int block_id,
                         int x_pos,
                         int y_pos);
RETURN_CODES remove_block(puzzle_def* puzzle,
                          int block_id,
                          int x_pos,
                          int y_pos);

int get_n_available_pieces(puzzle_def* puzzle, int block_id);

bool placement_resolvable(puzzle_def* puzzle,
                          int block_id,
                          int x_pos,
                          int y_pos);
bool is_puzzle_solved(puzzle_def* puzzle);

dynarr_head* get_puzzle_journal();
puz_entry* get_first_entry();
int get_puz_journal_size();
int get_puz_entry_size();
int get_puzzle_def_size();

void play_puzzle(puzzle_def* puzzle);
void print_grid(puzzle_def* puzzle, FILE* file_ptr);
void print_free_pieces(puzzle_def* puzzle, FILE* file_ptr);