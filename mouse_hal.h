/*
 * mouse_hal.h - ENCS4380 Micromouse Hardware Abstraction Layer
 * Birzeit University - Interfacing Techniques - first semester 2026/27
 *
 * DO NOT MODIFY THIS FILE. The official copy is used when your code is
 * graded; any change you make here is overwritten.
 *
 * Your maze solver (solver.c) talks to the mouse ONLY through the functions
 * below. The same solver.c is linked with:
 *   hal_sim.c      -> runs in the mms simulator (Phase 1)
 *   hal_esp32.cpp  -> runs on your robot (Phases 3 to 7; you write it)
 *
 * Coordinates: cell (0,0) is the start cell in the south-west corner.
 * x grows to the east, y grows to the north. The mouse starts in (0,0)
 * facing north. The HAL does NOT track position or heading: that is the
 * solver's job.
 */
#ifndef MOUSE_HAL_H
#define MOUSE_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MM_HAL_VERSION "ENCS4380-2026/27-S1"
#define MM_MAX_SIZE    16          /* largest maze side the HAL supports */

/* ---- Directions -------------------------------------------------------- */

/* Absolute headings. Turning right adds 1 (mod 4), turning left adds 3. */
enum { MM_NORTH = 0, MM_EAST = 1, MM_SOUTH = 2, MM_WEST = 3 };

/* Wall bit for an absolute direction, used in your map and in the map dump.
 * The bit order is specific to this semester (N=0x8, E=0x4, S=0x2, W=0x1).
 * Always write MM_WALL(dir); never hard-code the numbers. */
#define MM_WALL(dir)   ((uint8_t)(0x8u >> ((unsigned)(dir) & 3u)))
#define MM_ALL_WALLS   ((uint8_t)0x0F)

/* Bits returned by mm_sense(), RELATIVE to the mouse's current heading. */
#define MM_SEE_LEFT    0x1
#define MM_SEE_FRONT   0x2
#define MM_SEE_RIGHT   0x4

/* ---- Types ------------------------------------------------------------- */

typedef enum {
    MM_OK      = 0,   /* the move finished as asked                        */
    MM_BLOCKED = 1,   /* a wall stopped the move; the mouse did not move   */
    MM_FAULT   = 2    /* hardware or argument problem; stop and report it  */
} mm_status_t;

/* Maze size and goal block (inclusive cell coordinates). */
typedef struct {
    int width, height;
    int goal_x0, goal_y0;
    int goal_x1, goal_y1;
} mm_maze_t;

/* ---- The function YOU write -------------------------------------------- */

/* Entry point of your solver. Put it in solver.c. The simulator's main()
 * and the ESP32 sketch both call it. Call mm_begin() first inside it. */
void solver_run(void);

/* ---- Set-up ------------------------------------------------------------ */

/* Call once at the start of solver_run(). team_id is your team number as
 * on the sign-up sheet, e.g. "T07". */
void mm_begin(const char *team_id);

/* Maze size and goal. Never hard-code these: hidden test mazes use other
 * sizes and goal positions. */
mm_maze_t mm_maze(void);

/* ---- Sensing and motion ------------------------------------------------ */

/* Read the walls around the CURRENT cell, relative to the mouse:
 * returns any combination of MM_SEE_LEFT, MM_SEE_FRONT, MM_SEE_RIGHT.
 * Call it with the mouse stopped at a cell centre. */
uint8_t mm_sense(void);

/* Drive forward 'cells' whole cells (1 or more) and stop at the centre of
 * the last one. Returns MM_BLOCKED if a wall was in the way. */
mm_status_t mm_forward(int cells);

/* Turn in place by quarter turns: +1 = 90 deg right (clockwise),
 * -1 = 90 deg left, 2 = turn around. */
mm_status_t mm_turn(int quarter_turns);

/* ---- Restart after a touch --------------------------------------------- */

/* true if the operator restarted the mouse (simulator Reset button, or the
 * reset button on your robot). When it returns true: clear your position
 * (back to (0,0) facing north), KEEP your map, then call mm_reset_done(). */
bool mm_reset_requested(void);
void mm_reset_done(void);

/* ---- Showing what the solver thinks ------------------------------------ */
/* In the simulator these draw on the maze. On the robot they may do nothing. */

#define MM_COLOR_NONE    0
#define MM_COLOR_RED     'r'
#define MM_COLOR_GREEN   'g'
#define MM_COLOR_BLUE    'b'
#define MM_COLOR_YELLOW  'y'
#define MM_COLOR_ORANGE  'o'
#define MM_COLOR_CYAN    'c'
#define MM_COLOR_GRAY    'a'

void mm_mark(int x, int y, char color);         /* MM_COLOR_NONE clears    */
void mm_label(int x, int y, const char *text);  /* max 10 chars; NULL clears */
void mm_show_wall(int x, int y, int dir);       /* draw a wall you found   */
void mm_clear_marks(void);                      /* clear colours and text  */

/* ---- Logging and time -------------------------------------------------- */

/* printf-style debug message, one line. Goes to stderr in the simulator and
 * to Serial on the robot. NEVER use printf(): in the simulator, stdout is the
 * command channel and printing there breaks the run. */
void mm_log(const char *fmt, ...);

/* Milliseconds since mm_begin(). */
uint32_t mm_millis(void);

/* ---- Map dump (required) ----------------------------------------------- */
/* After each run, report your whole map so it can be checked against the
 * real maze. Call mm_map_begin(), then mm_map_cell() for every cell,
 * then mm_map_end().
 *   walls: MM_WALL() bits for the walls you believe exist
 *   known: MM_WALL() bits for the sides you have actually observed
 * A side not in 'known' is reported as unknown, whatever 'walls' says. */
void mm_map_begin(void);
void mm_map_cell(int x, int y, uint8_t walls, uint8_t known);
void mm_map_end(void);

#ifdef __cplusplus
}
#endif

#endif /* MOUSE_HAL_H */
