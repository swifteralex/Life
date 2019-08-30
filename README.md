# Life
A C++ Windows desktop application that simulates Conway's Game of Life.

![Life Demo](demo/demo.gif)

## Information

Conway's Game of Life is a cellular automaton played on an infinite grid of cells that can either be "dead" or "alive." Every cell in the grid interacts with its eight neighboring cells. At each step in time, the following rules are applied:
- Alive cells with fewer than two alive neighbors will die, as if by underpopulation.
- Alive cells with two or three neighbors will live onto the next generation.
- Alive cells with more than three neighbors will die, as if by overpopulation.
- Dead cells with exactly three neighbors will become alive, as if by reproduction.

## Instructions
To run the program, simply download and run the .exe within this repository.
- Right click and drag to move the grid around.
- Use the mousewheel to zoom in and out.
- Left click and drag while the simulation is paused to create alive cells.
- Press CTRL+Z to undo the last left click.
- Press SPACEBAR to pause/unpause the simulation.
- Press C to clear all currently alive cells.
- Press the right and left arrow keys to speed up and slow down the simulation.
