# ENCS4380 Micromouse Project - Team T12 (MAR3EE)

Autonomous robot navigation for ENCS4380 Interfacing Techniques, Birzeit University,
first semester 2026/27.

The maze solver is written once in C (`solver.c`) and talks to the mouse only through
the course HAL (`mouse_hal.h`). The same file runs in the mms simulator now and on our
ESP32 robot in later milestones.

## Repository contents

| File / folder | What it is | Written by |
| --- | --- | --- |
| `solver.c` | Our maze solver | Team T12 |
| `mazes/T12_C_small6.txt`, `.goal` | Our maze that a wall follower cannot solve | Team T12 |
| `mouse_hal.h` | The HAL interface (do not modify) | Instructor |
| `hal_sim.c`, `main_sim.c`, `mm_common.c`, `mm_internal.h` | Simulator HAL, logging and map dump | Instructor |
| `example_wallfollow.c` | HAL usage example (not a solution) | Instructor |
| `mazes/practice*.txt`, `.goal` | Practice mazes | Instructor |
| `esp32/micromouse/` | Arduino sketch for the robot (later milestones) | Instructor / Team T12 |
| `Makefile` | Simulator build | Instructor |

## Build and run in mms

Linux / Moodle VM:

```
make
```

In mms, add a mouse with:

- Directory: this folder
- Build command: `gcc -O2 -std=c99 -Wall -Wextra -o solver main_sim.c hal_sim.c mm_common.c solver.c`
- Run command: `./solver` (Windows: build with `-o solver.exe` and run `solver.exe` with its full path)

To test a goal that is not in the centre, copy the maze's `.goal` file to `mm_goal.txt`
in this folder. Delete `mm_goal.txt` afterwards.

## M1 - Simulation (tag `M1`)

### What the solver does

1. **Map and position.** One byte per cell for walls and one for the sides we have seen,
   using `MM_WALL(dir)` bits. A wall is always written on both cells that share it.
   The solver tracks x, y and heading after every move.
2. **Sensing.** `mm_sense()` is relative (left / front / right); it is converted to
   absolute walls using the current heading.
3. **Flood fill.** BFS with a queue (no recursion, the ESP32 stack is small). Every cell
   gets its number of steps to the target, shown on the maze with `mm_label()`.
4. **Explore, return, speed run.** The mouse explores to the goal, re-flooding at every
   cell, then returns to the start, then races to the goal on the shortest proven path,
   driving straight stretches with one `mm_forward(n)`.
5. **Unknown cells.** Unknown sides count as open while exploring (to try the shortest
   path) and as walls in the speed run (so it never hits a surprise wall).
6. **Wrong readings.** A new reading always replaces an old one, and if a move is
   blocked the wall is recorded and the next flood routes around it.
7. **Map dump.** `mm_map_begin()` / `mm_map_cell()` / `mm_map_end()` after every run;
   sides never seen are reported as unknown.

### Testing

- Tested on `practice4_1`, `practice8_1`, `practice8_2`, `practice8_offcentre`
  (with `mm_goal.txt`), `practice16_1` and our own maze: goal reached, back at the start,
  speed run completed, 0 wrong walls in the map dump.
- Builds with `-Wall -Wextra` and no warnings on Windows (MinGW) and Linux
  (Ubuntu 24.04, gcc 13.3).
- Every function was tested in mms on its own; all test screenshots are on our Trello
  board (list M1).

### Our maze: `mazes/T12_C_small6.txt`

6x6, goal (2,2)-(3,3). The goal is an island with one entrance and an open ring around
it, so its walls do not touch the outer frame. A left-hand wall follower loops around it
and never reaches the goal. Our flood-fill solver reaches it in 7 steps, and its speed run
(7 cells) equals the true shortest path.

## How we worked

All four of us wrote the solver together in shared-screen sessions.
We took turns typing: each member wrote parts of the code while the others
reviewed and tested, so everyone contributed to every part
(movement, map, flood fill and strategy).
Each function was tested in mms on its own before being merged, and every test
screenshot is on our Trello M1 card.

AI use: Claude was used as a tutor (explaining concepts and reviewing code) and helped
write and merge parts of the solver, with the instructor's approval. Every function was
explained, tested in mms and logged on Trello by the team.

## Team T12

- Name 1
- Name 2
- Name 3
- Name 4
