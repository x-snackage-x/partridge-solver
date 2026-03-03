# Partridge Puzzle Solver

A Partridge Puzzle solver inspired by the Stand-up Maths video ["The impossible puzzle with over a million solutions!"](https://www.youtube.com/watch?v=eqyuQZHfNPQ).

https://github.com/user-attachments/assets/3a07d338-07fe-4609-9ce8-4c470945a787

## Usage

The solver is implemented as a console program that can be called with three optional command line arguments. Passing an integer will be interpreted as the desired puzzle definition type i.e. the size of the largest tile. The flags `{vis|novis} {fulllog|nofulllog} {override|nooverride}` can activate the visualizer, the full log and overriding the protection for unsolvable puzzles.  
The defaults are: `8 novis nofulllog nooverride`  

```shell
wd$: ./sol.out {integer} {vis|novis} {fulllog|nofulllog} {override|nooverride}
```
Currently, the terminal visualizer will only work for grid sizes and terminal window sizes that don't result in scrolling behavior.

A `makefile` is provided for building the project as either a Linux or Windows application. 

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

Solver:  
- Everything lol  
Steps:
    1. ~~Integrate `puz.c` with `sol.c`: i.e. `sol.c` can play the puzzle~~ 
    1. Implement selection strategies
        - ~~Random with filter~~
        - ~~largest possible with filter~~
        - others?
    1. ~~Record moves in tree using `elhaylib.c`: i.e. `sol.c` can remember moves~~
    1. ~~Implement a `sol.c` function to scan if the board is in a solvable state~~
    1. ~~Implement line scanning solver function~~
    1. ~~Use function pointer approach to pass a callback function that visualizes the board~~
    1. Improve Memory Consumption:
        1. ~~Transition from Array of bools for `valid_tiles` to bit representation~~ -> Not sufficient, in testing still a limit of ~60mil nodes
        1. ~~Implement tree pruning and memory free algorithm for elhaylib and use it to prune explored branches and free memory to avoid OOM errors~~
    1. Implement more complex `is_solvable` algos?
    1. ~~Implement a print function to save a visual representation of tree to a file~~
    1. ~~If Tree to large print solution branch instead~~
    1. Expand to be able to pass a starting configutation
    1. Add command line arguments for controlling visualizer and printing
        - ~~puzzle size~~
        - ~~vis and novis~~
        - ~~fulllog and nofulllog~~
        - Set iteration limit or tree size limit?
    1. Improve code quality and cleanup
    1. ~~Add five more colors~~
    1. Fix VSC setup defaults for run and debug (seems like the run config/task/launch is missing maybe that's what's causing issues)

Visualizer  
- Utilize "Alternate Screen Buffer" to make visualizer robust to scrolling behaviour  
- Figure out if perfect squares are possible with moderate effort
- Implement a GUI using `raylib`/`raygui`

Portability
- ~~Port to Windows~~ 
- Port to WASM using `Emscripten`?

Puzzle
- ~~Make it playable using console commands~~
