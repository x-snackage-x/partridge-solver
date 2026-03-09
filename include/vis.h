#pragma once

#include <stdbool.h>

typedef enum COLOR {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
    ORANGE = 202,
    ROYAL_BLUE = 17,
    GRAY = 8,
    HINGREEN = 10,
    HINYELLOW = 11,
    HINBLUE = 12,
    PINK = 13,
    LIGRAY = 250,
    DARKGRAY = 235
} COLOR;

bool init_terminal(bool set_points);
void prep_vis_grid(int size, int puzzle_type);
void render_vis_grid(int size);
void reset_vis_grid(int size);
void record_vis_grid(int size);
void set_vis_block(int block_size, int x_pos, int y_pos);
void set_vis_block_color(int block_size, COLOR color, int x_pos, int y_pos);
void remove_vis_block(int block_size, int x_pos, int y_pos);

void def_block_colors(int* in_block_colors, int size);
COLOR get_block_color(int block_size);

bool clean_vis(bool clean_grid, int size);