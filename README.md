# Partridge Puzzle Solver

A Partridge Puzzle solver inspired by the Stand-up Maths video ["The impossible puzzle with over a million solutions!"](https://www.youtube.com/watch?v=eqyuQZHfNPQ). In addition to being an interesting problem to solve, this project was also an opportunity to use by personal generic data structures library [elhaylib](https://github.com/x-snackage-x/elhaylib) and vet it against a real use case. 

https://github.com/user-attachments/assets/3a07d338-07fe-4609-9ce8-4c470945a787

## Usage

The solver is implemented as a console program that can be called with four optional command line arguments. Passing an integer will be interpreted as the desired puzzle definition type i.e. the size of the largest tile. The flags `<vis|novis> <fulllog|nofulllog> <override|nooverride>` can activate the visualizer, the full log and overriding the protection for unsolvable puzzles.  
The defaults are: `8 novis nofulllog nooverride`  

```shell
wd$: ./sol_cli.out <integer> <vis|novis> <fulllog|nofulllog> <override|nooverride>
```

To pass a starting configuration by way of an input file save the definition under `start.in` placed in the execution directory and pass the command line argument `readin`. The puzzle definition input argument is omitted when using an input file. The file needs to conform to the following structure:

```C
8           // the puzzle definition type
7 0 0       // Tile placements with $Tile_type $X_position $Y_position
...         // etc.
``` 

```shell
wd$: ./sol_cli.out <readin> <vis|novis> <fulllog|nofulllog> <override|nooverride>
```

An example `start.in` is provided.

Currently, the terminal visualizer will only work for grid sizes and terminal window sizes that don't result in scrolling behavior.

## Build

A `makefile` is provided for building the project as either a Linux, Windows application or Emscripten-compiled WebAssembly binary and JavaScript interface file. The WebAssembly build includes only the core puzzle and solver logic without the CLI portion.

To build:

```shell
wd$: make <all|vis|puz|sol|sol_opt|sol_win|sol_WASM>
```

`make all`/`make` will build sol, sol_opt, sol_win and sol_WASM.

Note: To compile to WebAssembly make sure to activate Emscripten for the current terminal before building:

```shell
wd$:  source "[...]/emsdk_env.sh"
```

## Useful links puzzle

More info on the puzzle can be found here:

- The Partridge Puzzle by Robert T. Wainwright: https://www.mathpuzzle.com/partridge.html
- Matt Scroggs' blog on the subject: https://www.mscroggs.co.uk/blog/119
- Matt Scroggs' web interactive "Squares": https://www.mscroggs.co.uk/squares/
- Potapov Danila's blog about programming a solution finder: https://habr.com/ru/articles/889410/
- OEIS sequence for number of solutions: https://oeis.org/A381976


## Useful links project 

Sources that were useful while implementing the project.

WASM Stuff:
- https://developer.mozilla.org/en-US/docs/WebAssembly
- Emscripten, possible interactions between WASM and JS: https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#interacting-with-code-call-function-pointers-from-c
- Compiling C to WebAssembly without Emscripten: https://surma.dev/things/c-to-webassembly/

Visualizer Stuff:
- https://www.uninformativ.de/blog/postings/2016-12-17/0/POSTING-en.html i.e. use ▀▀
- https://notes.burke.libbey.me/ansi-escape-codes/

Makefile starting guide:
- https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## Open Tasks/Improvements:

Solver Improvements:
1. Implement additional more sophisticated `is_solvable` algos?
1. Improve code quality and cleanup

Dev. Env. ToDos:
1. Fix VSC setup defaults for run and debug (seems like the run config/task/launch is missing maybe that's what's causing issues)

Visualizer  
- Utilize "Alternate Screen Buffer" to make visualizer robust to scrolling behaviour  
- Figure out if perfect squares are possible with moderate effort
- Implement a GUI using `raylib`/`raygui`
