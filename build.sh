# build mandelbrot fractal visualizer
gcc -std=c89 -pedantic -o build/mandelbrot mandelbrot.c -lSDL2

# build tetris
as -o build/tetris.o tetris.s
ld -o build/tetris build/tetris.o
rm build/tetris.o
