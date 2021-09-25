Conway's Game of Life
=====================

An implementation of Conway's Game of Life on a 64 x 48 grid.
Created for the [2020 OLC Code Jam](https://itch.io/jam/olc-codejam-2020).
This is the first code jam / game jam I have ever entered, so at the advice of
javid9x I tried not to get too ambitious.
I figured that Conway's Game of Life fit the theme of "The Great Machine" well
enough due to the incredible complexities that can arise from the game's simple
rule set, ranging from gliders all the way up to working computer architectures
capable of playing Tetris[1].

My entry was developed using my own work-in-progress 2D engine with an SDL2
backend.
I currently do not have access to a Window or Mac development machine, so to
keep things cross platform I compiled the entire thing for the web with
Emscripten.

[1]: https://codegolf.stackexchange.com/questions/11880/build-a-working-game-of-tetris-in-conways-game-of-life

## Usage
```sh
$ ./life
```

## Dependencies
### Debian / Ubuntu
```
$ apt install libsdl2-dev
$ apt install libsdl2-image-dev
```

## Building
### POSIX c99
```sh
$ make life
```

### GCC or Clang (Linux)
```sh
$ make life CC=gcc CFLAGS='$(GNU_REL)'
$ make life CC=clang CFLAGS='$(GNU_REL)'
```

### Emscripten (Web)
#### Separate HTML, Js, and WASM files
```sh
$ make life.html
```
#### Single file
```sh
$ make life.bundle.html
```

After building, the web version of the application can be viewed by running:
```sh
$ python3 -m http.server 8080
```
and navigating to `http://localhost:8080/life.html` in a browser.

## License
This project and all of its content are released into the public domain under
the terms of BSD Zero Clause License (see LICENSE).
