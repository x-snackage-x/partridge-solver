IDIR=include
SDIR=src
CC=cc
CC_WIN = x86_64-w64-mingw32-gcc
DEBUG_FLAGS = -g3 -Og -fno-omit-frame-pointer
CFLAGS=-Wall $(DEBUG_FLAGS)
PROD_FLAGS = -O2
INC=-I$(IDIR)
LIBS=-lc

# Headers
_DEPS=elhaylib.h vis.h puz.h sol.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

# Default
all: sol sol_opt sol_win

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
        	$(SOL_ODIR)/sol.o

$(SOL_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_ODIR)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_ODIR):
	mkdir -p $@

sol: $(SOL_OBJS)
	$(CC) -o sol.out $^ $(LIBS)

# Prod Linux build 
sol_opt:CFLAGS = -Wall $(PROD_FLAGS)
SOL_OPT_ODIR=obj/sol_opt
SOL_OBJS= $(SOL_OPT_ODIR)/elhaylib.o \
		    $(SOL_OPT_ODIR)/vis.o \
		    $(SOL_OPT_ODIR)/puz.o \
		    $(SOL_OPT_ODIR)/sol.o

$(SOL_OPT_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_OPT_ODIR)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_OPT_ODIR):
	mkdir -p $@

sol_opt: $(SOL_OBJS)
	$(CC) -o sol_opt.out $^ $(LIBS)

# Prod MinGW Windows build:
sol_win:CFLAGS = -Wall $(PROD_FLAGS)
SOL_WIN_ODIR=obj/sol_mingw
SOL_OBJS= $(SOL_WIN_ODIR)/elhaylib.o \
		    $(SOL_WIN_ODIR)/vis.o \
		    $(SOL_WIN_ODIR)/puz.o \
		    $(SOL_WIN_ODIR)/sol.o

$(SOL_WIN_ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(SOL_WIN_ODIR)
	$(CC_WIN) -c $(INC) $(CFLAGS) $< -o $@

$(SOL_WIN_ODIR):
	mkdir -p $@

sol_win: $(SOL_OBJS)
	$(CC_WIN) -o sol.exe $^


# --------------------
clean:
	rm -rf obj *.out *.exe
