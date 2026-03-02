#include <stdio.h>
#include <stdlib.h>

#include <puz.h>

void init_puzzle(puzzle_def* puzzle, bool override) {
    if(!override && puzzle->size > 1 && puzzle->size < 8) {
        printf(
            "The Partridge puzzle has no solutions for sizes between(inc) 2 "
            "and 7.\n");
        printf("You have provided a puzzle size of : %d\n", puzzle->size);
        exit(EXIT_SUCCESS);
    }

    // init Blocks
    puzzle->blocks = calloc(1, sizeof(dynarr_head));
    puzzle->blocks->elem_size = sizeof(block_def);
    puzzle->blocks->dynarr_capacity = (size_t)(puzzle->size + 1);
    dynarr_init(puzzle->blocks);

    for(int i = 0; i <= puzzle->size; ++i) {
        block_def curr_block = {i, i};
        dynarr_append(puzzle->blocks, &curr_block);
    }

    // init Grid
    puzzle->grid_dimension = (puzzle->size * (puzzle->size + 1)) / 2;
    int grid_size = puzzle->grid_dimension;
    puzzle->puzzle_grid = calloc(grid_size, sizeof(int*));
    puzzle->puzzle_grid[0] = calloc(grid_size, grid_size * sizeof(int));
    for(int i = 1; i < grid_size; i++) {
        puzzle->puzzle_grid[i] = puzzle->puzzle_grid[0] + i * grid_size;
    }
}

int get_n_available_pieces(puzzle_def* puzzle_def, int block_id) {
    block_def* blocks = (block_def*)puzzle_def->blocks->ptr_first_elem;
    return blocks[block_id].free_pieces;
}

bool is_puzzle_solved(puzzle_def* puzzle) {
    int** grid = puzzle->puzzle_grid;
    for(int i = 0; i < puzzle->grid_dimension; ++i) {
        for(int j = 0; j < puzzle->grid_dimension; ++j) {
            if(grid[i][j] == 0) {
                return false;
            }
        }
    }

    return true;
}

bool placement_resolvable(puzzle_def* puzzle,
                          int block_id,
                          int x_pos,
                          int y_pos) {
    if(x_pos > puzzle->grid_dimension - 1 ||
       x_pos + block_id - 1 > puzzle->grid_dimension - 1 ||
       y_pos > puzzle->grid_dimension - 1 ||
       y_pos + block_id - 1 > puzzle->grid_dimension - 1) {
        return false;
    }
    int** grid = puzzle->puzzle_grid;
    for(int i = 0; i < block_id; ++i) {
        for(int j = 0; j < block_id; ++j) {
            if(grid[y_pos + i][x_pos + j] != 0) {
                return false;
            }
        }
    }

    return true;
}

RETURN_CODES place_block(puzzle_def* puzzle,
                         int block_id,
                         int x_pos,
                         int y_pos) {
    if(get_n_available_pieces(puzzle, block_id) <= 0) {
        return NO_FREE_PIECES;
    }

    if(!placement_resolvable(puzzle, block_id, x_pos, y_pos)) {
        return CONFLICT_ON_GRID;
    }

    int** grid = puzzle->puzzle_grid;
    for(int i = 0; i < block_id; ++i) {
        for(int j = 0; j < block_id; ++j) {
            grid[y_pos + i][x_pos + j] = block_id;
        }
    }
    block_def* blocks = (block_def*)puzzle->blocks->ptr_first_elem;
    --blocks[block_id].free_pieces;
    return SUCCESS;
}

RETURN_CODES remove_block(puzzle_def* puzzle,
                          int block_id,
                          int x_pos,
                          int y_pos) {
    int** grid = puzzle->puzzle_grid;
    if(grid[y_pos][x_pos] == 0) {
        return NO_BLOCK_AT_POSITION;
    }
    for(int i = 0; i < block_id; ++i) {
        for(int j = 0; j < block_id; ++j) {
            if(grid[y_pos + i][x_pos + j] != block_id) {
                return CONFLICTING_BLOCK_TYPES;
            }
        }
    }

    for(int i = 0; i < block_id; ++i) {
        for(int j = 0; j < block_id; ++j) {
            grid[y_pos + i][x_pos + j] = 0;
        }
    }
    block_def* blocks = (block_def*)puzzle->blocks->ptr_first_elem;
    ++blocks[block_id].free_pieces;
    return SUCCESS;
}

void print_grid(puzzle_def* puzzle, FILE* file_ptr) {
    if(file_ptr == NULL)
        file_ptr = stdout;

    int** grid = puzzle->puzzle_grid;
    for(int i = 0; i < puzzle->grid_dimension; ++i) {
        for(int j = 0; j < puzzle->grid_dimension; ++j) {
            fprintf(file_ptr, "%d|", grid[i][j]);
        }
        fprintf(file_ptr, "\n");
    }
}

void print_free_pieces(puzzle_def* puzzle, FILE* file_ptr) {
    if(file_ptr == NULL)
        file_ptr = stdout;

    block_def* my_blocks = (block_def*)puzzle->blocks->ptr_first_elem;
    for(int i = 0; i < puzzle->blocks->dynarr_capacity; ++i) {
        fprintf(file_ptr, "Block size: %d    Free pieces: %d\n",
                my_blocks->size, get_n_available_pieces(puzzle, i));
        ++my_blocks;
    }
}

#ifdef BUILD_PUZ
int main() {
    puzzle_def my_puzzle_def = {0};
    my_puzzle_def.size = 8;
    init_puzzle(&my_puzzle_def);

    printf("Print Grid:\n");
    print_grid(&my_puzzle_def, stdout);

    printf("---------------------------\n");

    printf("Printing Blocks:\n");
    printf("Block Dyn-array Cap: %ld - Block Dyn-array Size: %ld\n",
           my_puzzle_def.blocks->dynarr_capacity,
           my_puzzle_def.blocks->dynarr_size);
    print_free_pieces(&my_puzzle_def, stdout);

    printf("---------------------------\n");

    printf("Testing Conflict Function:\n");
    printf("No placements:\n");
    printf("Testing 4 @ (50,5): %d\n", place_block(&my_puzzle_def, 4, 50, 5));
    printf("Testing 4 @ (34,5): %d\n", place_block(&my_puzzle_def, 4, 34, 5));
    printf("Placing 4 @ (32,5): %d\n", place_block(&my_puzzle_def, 4, 32, 5));
    printf("Placing 7 @ (3,5): %d\n", place_block(&my_puzzle_def, 7, 3, 5));
    printf("Testing 4 @ (2,3): %d\n", place_block(&my_puzzle_def, 4, 2, 3));
    printf("Testing 4 @ (5,6): %d\n", place_block(&my_puzzle_def, 4, 5, 6));
    printf("Placing 1 @ (0,0): %d\n", place_block(&my_puzzle_def, 1, 0, 0));
    printf("Placing 1 @ (0,0): %d\n", place_block(&my_puzzle_def, 1, 0, 0));

    print_grid(&my_puzzle_def, stdout);

    printf("Removing 1 @ (0,0): %d\n", remove_block(&my_puzzle_def, 1, 0, 0));
    print_grid(&my_puzzle_def, stdout);

    printf("Removing 1 @ (0,0): %d\n", remove_block(&my_puzzle_def, 1, 0, 0));
    printf("Removing 5 @ (3,5): %d\n", remove_block(&my_puzzle_def, 5, 3, 5));
    printf("Removing 4 @ (32,5): %d\n", remove_block(&my_puzzle_def, 4, 32, 5));
    print_grid(&my_puzzle_def, stdout);

    printf("Is puzzle solved: %d\n", is_puzzle_solved(&my_puzzle_def));

    print_free_pieces(&my_puzzle_def, stdout);
}
#endif