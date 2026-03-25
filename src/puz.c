#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <puz.h>

dynarr_head* puz_journal;

dynarr_head* get_puzzle_journal() {
    return puz_journal;
}

puz_entry* get_first_entry() {
    return (puz_entry*)puz_journal->ptr_first_elem;
}

int get_puz_journal_size() {
    return (int)puz_journal->dynarr_size;
}

int get_puz_entry_size() {
    return (int)sizeof(puz_entry);
}

int get_puzzle_def_size() {
    return (int)sizeof(puzzle_def);
}

int find_journal_entry(puz_entry* needle) {
    puz_entry* ptr_journal = (puz_entry*)puz_journal->ptr_first_elem;
    size_t n_entries = puz_journal->dynarr_size;

    int found_index = -1;
    for(int i = 0; i < n_entries; ++i) {
        if(needle->type == ptr_journal->type &&
           needle->x_pos == ptr_journal->x_pos &&
           needle->y_pos == ptr_journal->y_pos) {
            found_index = i;
            break;
        } else {
            ++ptr_journal;
        }
    }

    return found_index;
}

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

    // init journal
    puz_journal = calloc(1, sizeof(dynarr_head));
    puz_journal->elem_size = sizeof(puz_entry);
    dynarr_init(puz_journal);
}

void free_puzzle(puzzle_def* puzzle) {
    dynarr_free(puz_journal);
    free(puz_journal);

    free(puzzle->puzzle_grid[0]);
    free(puzzle->puzzle_grid);

    dynarr_free(puzzle->blocks);
    free(puzzle->blocks);
}

int get_n_available_pieces(puzzle_def* puzzle_def, int block_id) {
    if(block_id > puzzle_def->size || block_id <= 0) {
        return 0;
    }

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

    puz_entry entry_buffer = {
        .type = block_id,
        .x_pos = x_pos,
        .y_pos = y_pos,
    };
    dynarr_append(puz_journal, &entry_buffer);

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
    puz_entry needle = {
        .type = block_id,
        .x_pos = x_pos,
        .y_pos = y_pos,
    };
    int index = find_journal_entry(&needle);
    if(index < 0) {
        return CORRESPONDING_MOVE_NOT_FOUND;
    }
    dynarr_remove(puz_journal, (size_t)index);

    for(int i = 0; i < block_id; ++i) {
        for(int j = 0; j < block_id; ++j) {
            grid[y_pos + i][x_pos + j] = 0;
        }
    }
    block_def* blocks = (block_def*)puzzle->blocks->ptr_first_elem;
    ++blocks[block_id].free_pieces;

    return SUCCESS;
}

void print_help() {
    printf("Place Block Format: ${Tile Type} ${X Pos} ${Y Pos} eg. 8 0 0\n");
    printf(
        "Remove Block Format: r ${Tile Type} ${X Pos} ${Y Pos} eg. r 8 0 0\n");
    printf("Quit input: q\n");
    printf("Print Grid: g\n");
    printf("Print available Tiles: t\n");
    printf("Print this help text: h\n");
}

void play_puzzle(puzzle_def* puzzle) {
    char buff[20];
    print_help();
    while(true) {
        char* fgbuf = fgets(buff, 10, stdin);
        if(fgbuf == NULL) {
            printf("Failure of fgets. Repeat input.\n");
        } else if(buff[0] == 'q') {
            break;
        } else if(buff[0] == 'g') {
            print_grid(puzzle, stdout);
            continue;
        } else if(buff[0] == 't') {
            print_free_pieces(puzzle, stdout);
            continue;
        } else if(buff[0] == 'h') {
            print_help();
            continue;
        } else {
            bool is_remove = false;
            char* token = strtok(buff, " ");
            if(token == NULL)
                continue;
            if(token[0] == 'r') {
                is_remove = true;
                token = strtok(NULL, " ");
                if(token == NULL)
                    continue;
            }
            int tile_type = (int)strtol(token, NULL, 10);
            token = strtok(NULL, " ");
            if(token == NULL)
                continue;
            int x_pos = (int)strtol(token, NULL, 10);
            token = strtok(NULL, " ");
            if(token == NULL)
                continue;
            int y_pos = (int)strtol(token, NULL, 10);

            RETURN_CODES answer;
            if(is_remove) {
                answer = remove_block(puzzle, tile_type, x_pos, y_pos);
            } else {
                answer = place_block(puzzle, tile_type, x_pos, y_pos);
            }

            switch(answer) {
                case SUCCESS:
                default:
                    break;
                case NO_FREE_PIECES:
                    printf("No free pieces of given type.\n");
                    break;
                case CONFLICT_ON_GRID:
                    printf("Conflict when placing tile.\n");
                    break;
                case CONFLICTING_BLOCK_TYPES:
                case NO_BLOCK_AT_POSITION:
                    printf("No or conflicting tile found at given position.\n");
                    break;
                case CORRESPONDING_MOVE_NOT_FOUND:
                    printf(
                        "No corresponding put move found for remove. Make sure "
                        "to input top-left position of block.\n");
                    break;
            }

            if(is_puzzle_solved(puzzle)) {
                print_grid(puzzle, stdout);
                printf("Solved configuration found\n");
                break;
            }
        }
    }
}

void print_grid(puzzle_def* puzzle, FILE* file_ptr) {
    int width = puzzle->size > 9 ? 2 : 1;

    if(file_ptr == NULL)
        file_ptr = stdout;

    int** grid = puzzle->puzzle_grid;
    for(int i = 0; i < puzzle->grid_dimension; ++i) {
        for(int j = 0; j < puzzle->grid_dimension; ++j) {
            fprintf(file_ptr, "%0*d|", width, grid[i][j]);
        }
        fprintf(file_ptr, "\n");
    }
}

void print_free_pieces(puzzle_def* puzzle, FILE* file_ptr) {
    if(file_ptr == NULL)
        file_ptr = stdout;

    block_def* my_blocks = (block_def*)puzzle->blocks->ptr_first_elem;
    for(int i = 0; i < puzzle->blocks->dynarr_capacity; ++i) {
        fprintf(file_ptr, "Block size: %2d    Free pieces: %2d\n",
                my_blocks->size, get_n_available_pieces(puzzle, i));
        ++my_blocks;
    }
}

#ifdef BUILD_PUZ
int main(int argc, char* argv[]) {
    puzzle_def my_puzzle_def = {0};
    my_puzzle_def.size = 8;

    if(argc > 1) {
        my_puzzle_def.size = (int)strtol(argv[1], NULL, 10);
    }
    printf("Puzzle definition: %d\n", my_puzzle_def.size);

    init_puzzle(&my_puzzle_def, true);
    play_puzzle(&my_puzzle_def);
    free_puzzle(&my_puzzle_def);
}
#endif