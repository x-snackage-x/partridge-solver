IDIR=include
SDIR=src
CC=cc
CC_WIN=x86_64-w64-mingw32-gcc
CC_WASM=emcc
DEBUG_FLAGS = -g3 -Og -fno-omit-frame-pointer
CFLAGS=-Wall $(DEBUG_FLAGS)
PROD_FLAGS = -O2
INC=-I$(IDIR)
LIBS=-lc

# Headers
_DEPS=elhaylib.h vis.h puz.h sol.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

# Default
all: sol sol_opt sol_win sol_WASM
sol_cli: sol

# --------------------
# VIS
# --------------------
VIS_ODIR=obj/vis
VIS_OBJS=$(VIS_ODIR)/elhaylib.o $(VIS_ODIR)/vis.o

$(VIS_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(VIS_ODIR)
	$(CC) -c $(INC) $(CFLAGS) -DBUILD_VIS $< -o $@

$(VIS_ODIR):
	mkdir -p $@

vis: $(VIS_OBJS)
	$(CC) -o vis.out $^ $(LIBS)

# --------------------
# PUZ
# --------------------
PUZ_ODIR=obj/puz
PUZ_OBJS=$(PUZ_ODIR)/elhaylib.o $(PUZ_ODIR)/puz.o

$(PUZ_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(PUZ_ODIR)
	$(CC) -c $(INC) $(CFLAGS) -DBUILD_PUZ $< -o $@

$(PUZ_ODIR):
	mkdir -p $@

puz: $(PUZ_OBJS)
	$(CC) -o puz.out $^ $(LIBS)

# --------------------
# SOL
# --------------------

# Dev build
SOL_ODIR=obj/sol
SOL_OBJS= $(SOL_ODIR)/elhaylib.o \
        	$(SOL_ODIR)/vis.o \
        	$(SOL_ODIR)/puz.o \
        	$(SOL_ODIR)/sol.o \
			$(SOL_ODIR)/sol_cli.o

$(SOL_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_ODIR)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_ODIR):
	mkdir -p $@

sol: $(SOL_OBJS)
	$(CC) -o sol_cli_dev.out $^ $(LIBS)

# Dev and progress_mode build
SOL_PROG_ODIR=obj/sol_prog
SOL_OBJS= $(SOL_PROG_ODIR)/elhaylib.o \
        	$(SOL_PROG_ODIR)/vis.o \
        	$(SOL_PROG_ODIR)/puz.o \
        	$(SOL_PROG_ODIR)/sol.o \
			$(SOL_PROG_ODIR)/sol_cli.o

$(SOL_PROG_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_PROG_ODIR)
	$(CC) -c $(INC) $(CFLAGS) -DBUILD_PROGRESS $< -o $@

$(SOL_PROG_ODIR):
	mkdir -p $@

sol_prog: $(SOL_OBJS)
	$(CC) -o sol_cli_prog.out -DBUILD_PROGRESS $^ $(LIBS)

# Prod Linux build 
sol_opt:CFLAGS=-Wall $(PROD_FLAGS)
SOL_OPT_ODIR=obj/sol_opt
SOL_OBJS= $(SOL_OPT_ODIR)/elhaylib.o \
		    $(SOL_OPT_ODIR)/vis.o \
		    $(SOL_OPT_ODIR)/puz.o \
		    $(SOL_OPT_ODIR)/sol.o \
			$(SOL_OPT_ODIR)/sol_cli.o

$(SOL_OPT_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_OPT_ODIR)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_OPT_ODIR):
	mkdir -p $@

sol_opt: $(SOL_OBJS)
	$(CC) -o sol_cli.out $^ $(LIBS)

# Prod MinGW Windows build:
sol_win:CFLAGS=-Wall $(PROD_FLAGS)
SOL_WIN_ODIR=obj/sol_mingw
SOL_OBJS= $(SOL_WIN_ODIR)/elhaylib.o \
		    $(SOL_WIN_ODIR)/vis.o \
		    $(SOL_WIN_ODIR)/puz.o \
		    $(SOL_WIN_ODIR)/sol.o \
			$(SOL_WIN_ODIR)/sol_cli.o

$(SOL_WIN_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_WIN_ODIR)
	$(CC_WIN) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_WIN_ODIR):
	mkdir -p $@

sol_win: $(SOL_OBJS)
	$(CC_WIN) -o sol_cli.exe $^

# Export to WASM
sol_WASM:CFLAGS=-Wall $(PROD_FLAGS) 										\
				-sEXPORTED_FUNCTIONS=_malloc,_free,_init_puzzle,$\
					_free_puzzle,_place_block,_remove_block,$\
					_get_n_available_pieces,_placement_resolvable,$\
					_is_puzzle_solved,_get_first_entry,$\
					_get_puz_entry_size,_get_puzzle_def_size,$\
					_get_puz_journal_size,_print_grid,$\
					_print_free_pieces,_setup,_set_visualizer,$\
					_visualizer_on,_visualizer_off,_solution_search			\
				-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,setValue,getValue,$\
				 addFunction												\
				-sALLOW_TABLE_GROWTH -sMODULARIZE=1 -sEXPORT_ES6=1 

SOL_WEB_ODIR=obj/sol_web
SOL_SRCS= $(SDIR)/elhaylib.c \
		  	$(SDIR)/puz.c \
			$(SDIR)/sol.c \

$(SOL_WEB_ODIR):
	mkdir -p $@

sol_WASM: | $(SOL_WEB_ODIR)
	$(CC_WASM) $(SOL_SRCS) $(INC) $(LIBS) -o $(SOL_WEB_ODIR)/solWASM.js $(CFLAGS)


# --------------------
clean:
	rm -rf obj *.out *.exe
