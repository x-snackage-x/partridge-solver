#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <vis.h>
#define STEP_SIZE 2

COLOR* set_block_colors;
int size;

void def_block_colors(int* in_block_colors, int size) {
    set_block_colors = malloc(sizeof(int) * size);
    memcpy(set_block_colors, in_block_colors, sizeof(int) * size);
}

COLOR get_block_color(int block_size) {
    return *(set_block_colors + block_size - 1);
}

void set_vis_block(int block_size, int x_pos, int y_pos) {
    COLOR color = get_block_color(block_size);
    set_vis_block_color(block_size, color, x_pos, y_pos);
}
void set_vis_block_color(int block_size, COLOR color, int x_pos, int y_pos) {
    printf("\x1b[s");

    int x_shift = x_pos != 0 ? STEP_SIZE * x_pos + 1 : x_pos;
    int y_shift = y_pos;
    if(x_shift != 0) {
        printf("\x1b[%dG", x_shift);
    }
    if(y_shift != 0) {
        printf("\x1b[%dB", y_shift);
    }

    for(int i = 0; i < block_size; ++i) {
        for(int j = 0; j < block_size; ++j) {
            printf("\x1b[38;5;%dm\x1b[48;5;%dm▀▀\x1b[0m", color, color);
        }
        printf("\x1b[1B\x1b[%dG", x_shift);
    }
    printf("\x1b[u");
}

void remove_vis_block(int block_size, int x_pos, int y_pos) {
    printf("\x1b[s");

    int x_shift = x_pos != 0 ? STEP_SIZE * x_pos + 1 : x_pos;
    int y_shift = y_pos;
    if(x_shift != 0) {
        printf("\x1b[%dG", x_shift);
    }
    if(y_shift != 0) {
        printf("\x1b[%dB", y_shift);
    }

    for(int i = 0; i < block_size; ++i) {
        for(int j = 0; j < block_size; ++j) {
#ifdef TESTING
            printf("..");
#else
            printf("  ");
#endif
        }
        printf("\x1b[1B\x1b[%dG", x_shift);
    }

    printf("\x1b[u");
}

void render_vis_grid(int size) {
    printf("\x1b[%dB", size);
    fflush(stdout);
}

bool init_terminal() {
    bool is_successful = true;

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hOut == INVALID_HANDLE_VALUE) {
        is_successful = false;
    }
    DWORD dwMode = 0;
    if(!GetConsoleMode(hOut, &dwMode)) {
        is_successful = false;
    }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if(!SetConsoleMode(hOut, dwMode)) {
        is_successful = false;
    }

    if(!is_successful) {
        printf("Couldn't enable virtual terminal processing.\n");
        printf("Disabling Visualizer.\n");
        return false;
    }

    SetConsoleOutputCP(CP_UTF8);
#endif

    return is_successful;
}

void prep_vis_grid(int size, int puzzle_type) {
    // Default colors
    COLOR blocks[] = {WHITE,     ROYAL_BLUE, ORANGE,   MAGENTA, CYAN, RED,
                      GREEN,     GRAY,       DARKGRAY, YELLOW,  BLUE, HINGREEN,
                      HINYELLOW, HINBLUE,    PINK,     LIGRAY};
    def_block_colors((int*)blocks, puzzle_type);

    for(int line = 0; line < size; ++line) {
#ifdef TESTING
        for(int i = 0; i < STEP_SIZE * size; i++) {
            putchar('.');
        }
        putchar('\n');
#else
        printf("%*c\n", STEP_SIZE * size, ' ');
#endif
    }

    printf("\x1b[%dA", size);
}

void reset_vis_grid(int size) {
    printf("\x1b[%dA", size);
}

void record_vis_grid(int size) {
    printf("\x1b[%dB\n", size);
}

#ifdef BUILD_VIS
int main() {
    int grid_size = 45;

    prep_vis_grid(grid_size);
    set_vis_block_color(2, GREEN, 1, 0);
    set_vis_block_color(1, WHITE, 0, 0);
    set_vis_block_color(2, ROYAL_BLUE, 5, 2);
    set_vis_block_color(5, GRAY, 5, 5);
    render_vis_grid(grid_size);
    reset_vis_grid(grid_size);

    usleep(25 * 100000);

    prep_vis_grid(grid_size);
    set_vis_block_color(2, GREEN, 1, 0);
    set_vis_block_color(1, WHITE, 0, 0);
    set_vis_block_color(2, BLUE, 5, 2);
    render_vis_grid(grid_size);
    reset_vis_grid(grid_size);

    usleep(25 * 100000);

    prep_vis_grid(grid_size);
    set_vis_block_color(2, GREEN, 1, 0);
    set_vis_block_color(1, WHITE, 0, 0);
    set_vis_block_color(2, BLUE, 5, 2);
    set_vis_block_color(5, GRAY, 5, 5);
    render_vis_grid(grid_size);
    reset_vis_grid(grid_size);

    usleep(25 * 100000);

    remove_vis_block(1, 0, 0);
    render_vis_grid(grid_size);
    reset_vis_grid(grid_size);

    usleep(25 * 100000);

    COLOR blocks[] = {ROYAL_BLUE, ORANGE, MAGENTA, CYAN, RED};
    def_block_colors((int*)blocks, 5);
    prep_vis_grid(grid_size);
    set_vis_block_color(2, GREEN, 1, 0);
    set_vis_block_color(1, WHITE, 0, 0);
    set_vis_block_color(2, BLUE, 5, 2);
    set_vis_block_color(5, GRAY, 5, 5);
    set_vis_block(3, 1, 20);
    set_vis_block(1, 0, 10);
    set_vis_block(2, 5, 12);
    set_vis_block(5, 5, 15);
    set_vis_block(4, 5, 20);
    render_vis_grid(grid_size);

    usleep(2 * 1000000);
}
#endif