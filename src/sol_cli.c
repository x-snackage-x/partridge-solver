#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/stat.h>

#include <limits.h>
#include <puz.h>
#include <sol.h>
#include <vis.h>

FILE* log_fptr;
FILE* tree_fptr;

bool full_log;
bool override;
extern bool visualizer_set;

extern puzzle_def* my_puzzle;

extern tree_head placement_record;
extern tree_node* last_placement;

extern bool is_solvable;
extern bool is_solved;
extern uint64_t loop_n;
extern uint64_t tree_max;

puzzle_def* setup_puzzle(int puzzle_type, FILE** in_ptr_ptr);
void handle_input(int argc, char** argv, int* puzzle_type, FILE** in_fptr);
int is_integer(const char* arg);
void printWinningBranch(FILE* file_ptr);
void printNode(tree_node* ptr_node, FILE* file_ptr);
void printTree(tree_node* ptr_node,
               int depth,
               bool isLast,
               bool* flag,
               FILE* file_ptr);
void log_tile_put_ok(int tile_type);
void log_tile_put_nok(int tile_type);
void log_tile_remove(int tile_type, int x_pos, int y_pos);

int main(int argc, char* argv[]) {
    full_log = false;
    visualizer_set = false;
    override = false;
    int puzzle_type = 8;

    FILE* in_ptr = NULL;
    handle_input(argc, argv, &puzzle_type, &in_ptr);

    // Make logs dir
    struct stat st = {0};
    if(stat("logs", &st) == -1) {
#ifdef __linux__
        mkdir("logs", 0700);
#elif _WIN32
        mkdir("logs");
#endif
    }

    // Open a file in writing mode
    log_fptr = fopen("logs/log.txt", "w");
    tree_fptr = fopen("logs/tree.txt", "w");

    puzzle_def* start_puzzle = setup_puzzle(puzzle_type, &in_ptr);
    setup(start_puzzle);

    if(in_ptr != NULL) {
        printf("Starting Configuration defined:\n");
        fprintf(log_fptr, "Starting Configuration defined:\n");
        print_grid(my_puzzle, NULL);
        print_grid(my_puzzle, log_fptr);
        printf("\n");
        fprintf(log_fptr, "\n");
    }

    if(visualizer_set) {
        visualizer_set = init_terminal(true);
    }

    if(visualizer_set) {
        prep_vis_grid(my_puzzle->grid_dimension, my_puzzle->size);

        // render start config if set
        dynarr_head* puzzle_journal = get_puzzle_journal();
        puz_entry* ptr_journal = (puz_entry*)puzzle_journal->ptr_first_elem;
        for(int i = 0; i < puzzle_journal->dynarr_size; ++i) {
            int x_pos = ptr_journal->x_pos;
            int y_pos = ptr_journal->y_pos;
            int type = ptr_journal->type;
            set_vis_block_color(type, BLACK, x_pos, y_pos);
            ++ptr_journal;
        }

        render_vis_grid(my_puzzle->grid_dimension);
        reset_vis_grid(my_puzzle->grid_dimension);

        set_visualizer(render_vis_grid, reset_vis_grid, set_vis_block,
                       remove_vis_block);
    }

#ifdef _WIN32
    Sleep(0.5);
#else
    usleep(5 * 10000);
#endif

    if(full_log) {
        set_logger(log_tile_put_ok, log_tile_put_nok, log_tile_remove);
    }

    // enable thousand separator
    setlocale(LC_NUMERIC, "");

    clock_t begin = clock();

    is_solvable = solution_search();

    clock_t end = clock();
    double solve_time = (double)(end - begin) / CLOCKS_PER_SEC;

    if(is_solved && visualizer_set) {
        record_vis_grid(my_puzzle->grid_dimension);
    }
    if(visualizer_set) {
        clean_vis(!is_solved, my_puzzle->grid_dimension);
    }

    fprintf(log_fptr, "Puzzle Status: Solvable: %s - Solved: %s\n\n",
            is_solvable ? "true" : "false", is_solved ? "true" : "false");
    printf("\33[2K\rPuzzle Status: Solvable: %s - Solved: %s\n",
           is_solvable ? "true" : "false", is_solved ? "true" : "false");
    printf("\33[2K\r\n");

    if(is_solved) {
        print_grid(my_puzzle, NULL);
        print_grid(my_puzzle, log_fptr);
        printf("\n");
    }
    print_free_pieces(my_puzzle, NULL);
    fprintf(log_fptr, "\n");
    print_free_pieces(my_puzzle, log_fptr);

    printf("\nMax. Tree Size: %'" PRIu64 " - Cur. Tree Size: %'zu Nodes\n",
           tree_max, placement_record.tree_size);
    fprintf(tree_fptr,
            "Max. Tree Size: %'" PRIu64 " - Cur. Tree Size: %'zu Nodes\n",
            tree_max, placement_record.tree_size);
    fprintf(log_fptr,
            "\nMax. Tree Size: %'" PRIu64 " - Cur. Tree Size: %'zu Nodes\n",
            tree_max, placement_record.tree_size);

    printf("n-Iterations: %'" PRIu64 "\n", loop_n);
    fprintf(log_fptr, "n-Iterations: %'" PRIu64 "\n", loop_n);
    printf("Solve Time: %f seconds\n", solve_time);
    fprintf(log_fptr, "Solve Time: %f seconds\n", solve_time);

    bool* flags = malloc(sizeof(bool) * placement_record.tree_size);
    memset(flags, true, placement_record.tree_size);

    printWinningBranch(tree_fptr);
    printTree(placement_record.tree_root, 0, false, flags, tree_fptr);

    // Close the files and free memory
    fclose(log_fptr);
    fclose(tree_fptr);

    free(flags);
    free(my_puzzle);

    return EXIT_SUCCESS;
}

puzzle_def* setup_puzzle(int puzzle_type, FILE** in_ptr_ptr) {
    puzzle_def* my_puzzle = calloc(1, sizeof(puzzle_def));
    my_puzzle->size = puzzle_type;
    init_puzzle(my_puzzle, override);

    if(*in_ptr_ptr == NULL) {
        return my_puzzle;
    }

    char line[20];
    FILE* in_file = *in_ptr_ptr;
    RETURN_CODES answer;
    while(fgets(line, 20, in_file)) {
        char* token = strtok(line, " ");
        int tile_type = (int)strtol(token, NULL, 10);
        token = strtok(NULL, " ");
        if(token == NULL)
            exit(EXIT_FAILURE);
        int x_pos = (int)strtol(token, NULL, 10);
        token = strtok(NULL, " ");
        if(token == NULL)
            exit(EXIT_FAILURE);
        int y_pos = (int)strtol(token, NULL, 10);

        answer = place_block(my_puzzle, tile_type, x_pos, y_pos);
        if(answer != SUCCESS) {
            printf(
                "Issue encountered placing tiles. Check tile placements in "
                "file for consistency.\n");
            exit(EXIT_FAILURE);
        }
    }

    fclose(in_file);

    return my_puzzle;
}

void handle_input(int argc, char** argv, int* puzzle_type, FILE** in_ptr_ptr) {
    bool integer_inputted = false;
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "vis") == 0) {
            visualizer_set = true;
        } else if(strcmp(argv[i], "fulllog") == 0) {
            full_log = true;
        } else if(strcmp(argv[i], "override") == 0) {
            override = true;
        } else if(strcmp(argv[i], "readin") == 0) {
            char line[20];
            *in_ptr_ptr = fopen("start.in", "r");
            if(*in_ptr_ptr == NULL)
                exit(EXIT_FAILURE);

            char* line_ptr = fgets(line, 20, *in_ptr_ptr);
            if(line_ptr == NULL) {
                printf("Input configuration file is empty.\n");
                exit(EXIT_FAILURE);
            } else if(strlen(line) > 3) {
                printf("First record isn't in the expected format.\n");
                exit(EXIT_FAILURE);
            }
            *puzzle_type = (int)strtol(line, NULL, 10);
            if(*puzzle_type > 16) {
                printf(
                    "Puzzle definitions greater than 16 aren't "
                    "permitted/useful.\n");
                exit(EXIT_FAILURE);
            }
            integer_inputted = true;

            continue;
        } else if(strcmp(argv[i], "novis") == 0 ||
                  strcmp(argv[i], "nofulllog") == 0 ||
                  strcmp(argv[i], "nooverride") == 0) {
            continue;
        } else if(strcmp(argv[i], "-h") == 0) {
            printf(
                "Usage: ./sol.out {number} {vis|novis} {fulllog|nofulllog} "
                "{override|nooverride}\n"
                "Command Line arguments are optional\n"
                "Defaults: 8 novis nofulllog.\n\n"
                "Usage with input start config in \"start.in\" file: ./sol.out "
                "{readin} {vis|novis} {fulllog|nofulllog} "
                "{override|nooverride}\n");
            return exit(EXIT_SUCCESS);
        } else if(is_integer(argv[i]) != 0 && !integer_inputted) {
            int num = (int)strtol(argv[1], NULL, 10);
            if(num < 0) {
                printf("A puzzle cannot be defined with a negative number.\n");
                return exit(EXIT_FAILURE);
            } else if(num > 16) {
                printf(
                    "Trying to descend to a solution for a puzzle of type "
                    "greater than 16 is highly ill advised.\nTrying to find "
                    "one for a size greater than 10 is already excessive.\n\n "
                    "I have you seen RAM prices lately? I doubt you have "
                    "enough memory on your personal machine to try and find a "
                    "solution\nfor anything greater than 9 anyway.\n\nIf "
                    "you're running this on some kind of computing server, I "
                    "ask why?\n\nI refuse to entertain your absurd "
                    "demands.\n\n(I also only allocated enough memory (16 "
                    "bits) for representing\nmaximally 16 different tile "
                    "types, so there's that)\n");
                return exit(EXIT_FAILURE);
            }
            *puzzle_type = num;
            integer_inputted = true;
        } else if(is_integer(argv[i]) != 0 && integer_inputted) {
            printf("Only one integer permited as input.\n");
            return exit(EXIT_FAILURE);
        } else {
            printf(
                "Command Line argument not recognized: only $number, "
                "vis/novis, fulllog/nofulllog override/nooverride are "
                "accepted.\n"
                "Usage example: ./sol.out 8 vis nofulllog\n");
            return exit(EXIT_FAILURE);
        }
    }
}

int is_integer(const char* arg) {
    char* endptr;
    errno = 0;
    long value = strtol(arg, &endptr, 10);

    // Check for various errors:
    // 1. No conversion occurred (empty string or not a number)
    if(arg == endptr || *endptr != '\0') {
        return 0;
    }
    // 2. Out of the range of a 'long'
    if(errno == ERANGE) {
        return 0;
    }
    // 3. Out of the range of an 'int' (if specifically needed)
    if(value > INT_MAX || value < INT_MIN) {
        return 0;
    }

    return 1;
}

void printWinningBranch(FILE* file_ptr) {
    int extra_spaces = my_puzzle->size - 8;
    extra_spaces = extra_spaces < 0 ? 0 : extra_spaces;
    int extra_spaces_l = extra_spaces / 2 + extra_spaces % 2;
    int extra_spaces_r = extra_spaces / 2;

    tree_node* current_node = last_placement;
    node_placement placement_data = {0};
    uint16_t tiles_mask;
    bool ascending = true;
    while(ascending) {
        placement_data = *(node_placement*)current_node->data;

        tiles_mask = placement_data.valid_tiles;

        fprintf(file_ptr, "┌──────────────────────────");
        for(int i = 0; i < extra_spaces; i++)
            fprintf(file_ptr, "─");
        fprintf(file_ptr, "┐\n");
        fprintf(file_ptr, "│ Tile: %2d %*s-%*s Pos.:(",
                placement_data.tile_type, extra_spaces_l, "", extra_spaces_r,
                "");
        if(placement_data.x_pos == 255) {
            fprintf(file_ptr, " -,");
        } else {
            fprintf(file_ptr, "%2d,", placement_data.x_pos);
        }
        if(placement_data.y_pos == 255) {
            fprintf(file_ptr, " - ) │\n");
        } else {
            fprintf(file_ptr, " %2d) │\n", placement_data.x_pos);
        }
        fprintf(file_ptr, "│ Valid Tiles:   ");
        for(int i = 0; i < my_puzzle->size; ++i) {
            fprintf(file_ptr, "%d", (tiles_mask >> i) & 1);
        }
        if(my_puzzle->size < 8) {
            fprintf(file_ptr, "%*s", 8 - my_puzzle->size, "");
        }
        fprintf(file_ptr, "  │\n");
        fprintf(file_ptr, "└──────────────────────────");
        for(int i = 0; i < extra_spaces; i++)
            fprintf(file_ptr, "─");
        fprintf(file_ptr, "┘\n");

        if(current_node->parent != NULL) {
            fprintf(file_ptr, "             %*s∧\n", extra_spaces_l, "");
            fprintf(file_ptr, "            %*s/ \\\n", extra_spaces_l, "");
            fprintf(file_ptr, "             %*s│\n", extra_spaces_l, "");
            fprintf(file_ptr, "             %*s│\n", extra_spaces_l, "");
        } else {
            ascending = false;
        }

        current_node = current_node->parent;
    }
}

void printNode(tree_node* ptr_node, FILE* file_ptr) {
    node_placement placement_data = *(node_placement*)ptr_node->data;
    uint16_t tiles_mask = placement_data.valid_tiles;
    fprintf(file_ptr, "[Tile: %d - Pos.:(%2d,%2d) ", placement_data.tile_type,
            placement_data.x_pos, placement_data.y_pos);
    fprintf(file_ptr, "Valid Tiles: ");
    for(int i = 0; i < my_puzzle->size; ++i) {
        fprintf(file_ptr, "%d", (tiles_mask >> i) & 1);
    }
    fprintf(file_ptr, "]\n");
}

// adapted from:
// https://www.geeksforgeeks.org/dsa/print-n-ary-tree-graphically/
void printTree(tree_node* ptr_node,
               int depth,
               bool isLast,
               bool* flag,
               FILE* file_ptr) {
    if(ptr_node == NULL) {
        fprintf(file_ptr, "[]\n");
        return;
    }

    // Loop to print the depths of the
    // current node
    for(int i = 1; i < depth; ++i) {
        // Condition when the depth
        // is exploring
        if(flag[i] == true) {
            fprintf(file_ptr, "│   ");
        }
        // Otherwise print
        // the blank spaces
        else {
            fprintf(file_ptr, "    ");
        }
    }

    // Condition when the current
    // node is the root node
    if(depth == 0) {
        printNode(ptr_node, file_ptr);
    } else if(isLast) {
        // Condition when the node is
        // the last node of
        // the exploring depth
        fprintf(file_ptr, "└───");
        printNode(ptr_node, file_ptr);
        // No more childrens turn it
        // to the non-exploring depth
        flag[depth] = false;
    } else {
        fprintf(file_ptr, "├───");
        printNode(ptr_node, file_ptr);
    }

    tree_node** pointer_to_children_pointers =
        (tree_node**)ptr_node->children.ptr_first_elem;
    tree_node* child_node_ptr = *pointer_to_children_pointers;

    for(size_t i = 0; i != ptr_node->children.dynarr_size; ++i) {
        // Recursive call for the
        // children nodes
        bool isLastChild = i == ptr_node->children.dynarr_size - 1;
        printTree(child_node_ptr, depth + 1, isLastChild, flag, file_ptr);

        child_node_ptr = *(++pointer_to_children_pointers);
    }
    flag[depth] = true;
}

void log_tile_put_ok(int tile_type) {
    fprintf(log_fptr, "Current tile: %d - Placement success: true\n",
            tile_type);
}

void log_tile_put_nok(int tile_type) {
    fprintf(log_fptr, "Current tile: %d - Placement success: false\n",
            tile_type);
}

void log_tile_remove(int tile_type, int x_pos, int y_pos) {
    fprintf(log_fptr, " Remove tile: %d, Pos. (%2d,%2d)\n", tile_type, x_pos,
            y_pos);
}
