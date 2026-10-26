/*
 * solver.c - Team T?? - robot name: ????
 *
 * Your maze solver. This file must:
 *   - include only mouse_hal.h and standard C headers (no simulator API,
 *     no Arduino or ESP32 headers),
 *   - talk to the mouse only through mm_... functions,
 *   - never print with printf(); use mm_log() instead,
 *   - never assume the maze size or goal; read them from mm_maze().
 * The same file must build unchanged for the simulator and for the ESP32.
 */
#include "mouse_hal.h"

void solver_run(void)
{
    mm_begin("T00");                       /* TODO: your team ID, e.g. "T07" */

    mm_maze_t maze = mm_maze();
    mm_log("solver started: maze %dx%d", maze.width, maze.height);

    /* M1 step 1 (week of 25 Oct): map and position
     *   - a wall map for every cell, using MM_WALL(dir) bits
     *   - track x, y and heading after every mm_forward() / mm_turn()
     *   - convert mm_sense() (relative) into absolute walls
     *
     * M1 step 2 (week of 1 Nov): flood fill with a queue, show the
     *   distances with mm_label()
     *
     * M1 step 3 (week of 8 Nov): explore to the goal, return to the
     *   start, then run the shortest known path
     *
     * M1 step 4 (week of 15 Nov): report your map with mm_map_begin(),
     *   mm_map_cell() for every cell, mm_map_end()
     */
}
