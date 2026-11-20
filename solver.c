/*
 * solver.c - Team T12 - robot name: MAR3EE
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
#include <stdio.h>     /* snprintf, for show_dist */
static mm_maze_t g_maze;
static int g_x, g_y, g_heading;   /* where the mouse is and where it faces */

static const int DX[4] = { 0, 1, 0, -1 };
static const int DY[4] = { 1, 0, -1, 0 };

/* the map: one byte per cell, using MM_WALL(dir) bits (N=0x8 E=0x4 S=0x2 W=0x1).
 * g_walls: sides that have a wall.
 * g_known: sides we have actually seen (wall OR open).
 * A side not in g_known is unknown. Index order is [x][y]. */
static uint8_t g_walls[MM_MAX_SIZE][MM_MAX_SIZE];
static uint8_t g_known[MM_MAX_SIZE][MM_MAX_SIZE];

/* flood fill result: steps from each cell to the target.
 * DIST_INF means "no known path". Written only by flood(). */
#define DIST_INF 9999
static int g_dist[MM_MAX_SIZE][MM_MAX_SIZE];

//functions
static int turn_dir(int dir, int quarter_turns){
    // +4 bcz neg number
    // %4 to save range in 0 to 3 bcz its just 4 dir
    return (((dir + quarter_turns)%4)+4)%4;
}

static int opposite(int dir)
{
    /* opposite side: the east wall of a cell is the west wall of its
     * east neighbour, so set_side needs the reverse direction */
    return turn_dir(dir, 2);
}
static bool in_maze(int x, int y){
    /* true if (x,y) is a real cell; we check before touching a
    * neighbour so we never read/write outside the arrays */
    return (x>=0 && y>=0 && x < g_maze.width && y < g_maze.height);

}

static mm_status_t do_turn(int quarter_turns){
    mm_status_t st = mm_turn(quarter_turns);

    if (st == MM_OK) {
        g_heading = turn_dir(g_heading, quarter_turns);
    } else {
        mm_log("turn failed (status %d) at (%d,%d) heading %d",
               st, g_x, g_y, g_heading);
    }

    return st;
}

static mm_status_t do_forward(int cells){
    mm_status_t st = mm_forward(cells);

    if (st == MM_OK) {
        g_x = g_x + DX[g_heading] * cells;
        g_y = g_y + DY[g_heading] * cells;
    } else {
        mm_log("forward failed (status %d) at (%d,%d)", st, g_x, g_y);
    }

    return st;
}

static mm_status_t face(int dir){
    int diff = ((dir - g_heading) % 4 + 4) % 4;

    if (diff == 0) return MM_OK;          /* already facing dir   */
    if (diff == 1) return do_turn(+1);    /* right                */
    if (diff == 2) return do_turn(2);     /* turn around          */
    return do_turn(-1);                   /* diff == 3: left      */
}

/* record one side of cell (x,y) as wall or open.
 * The same wall is also a side of the neighbour cell, so we write it on
 * BOTH cells. A new reading replaces an old one, so a wrong reading
 * can be fixed later. Only this function writes g_walls / g_known. */
static void set_side(int x, int y, int dir, bool is_wall){
    if (!in_maze(x, y)) return;               /* never write outside the arrays */

    /* --- this cell --- */
    uint8_t bit = MM_WALL(dir);                 /* e.g. east -> 0x4 */
    g_known[x][y] |= bit;                       /* we have seen this side */
    if (is_wall) g_walls[x][y] |= bit;          /* add the wall */
    else         g_walls[x][y] &= (uint8_t)~bit;/* remove it: side is open */

    /* --- the neighbour on the other side of the same wall --- */
    int nx = x + DX[dir];
    int ny = y + DY[dir];
    if (in_maze(nx, ny)) {                      /* outer walls have no neighbour */
        uint8_t obit = MM_WALL(opposite(dir));  /* east of us = west of them */
        g_known[nx][ny] |= obit;
        if (is_wall) g_walls[nx][ny] |= obit;
        else         g_walls[nx][ny] &= (uint8_t)~obit;
    }

    if (is_wall) mm_show_wall(x, y, dir);       /* draw it in mms (debug only) */
}

/* the outer frame is always walled, and we know the size from mm_maze(),
 * so record it before the mouse moves. Call once, NOT after a reset. */
static void init_map(void){
    for (int x = 0; x < g_maze.width; x++) {
        set_side(x, 0, MM_SOUTH, true);                  /* bottom row */
        set_side(x, g_maze.height - 1, MM_NORTH, true);  /* top row    */
    }
    for (int y = 0; y < g_maze.height; y++) {
        set_side(0, y, MM_WEST, true);                   /* left column  */
        set_side(g_maze.width - 1, y, MM_EAST, true);    /* right column */
    }
}

/* read the 3 sensors and store what they see in the map.
 * mm_sense() is RELATIVE to the mouse (left/front/right), the map is
 * ABSOLUTE (N/E/S/W), so we turn each one into an absolute direction
 * using the current heading. Call it with the mouse stopped at a cell centre. */
static void sense_and_record(void){
    uint8_t s = mm_sense();

    set_side(g_x, g_y, turn_dir(g_heading, -1), (s & MM_SEE_LEFT)  != 0);  /* left  */
    set_side(g_x, g_y, g_heading,               (s & MM_SEE_FRONT) != 0);  /* front */
    set_side(g_x, g_y, turn_dir(g_heading, +1), (s & MM_SEE_RIGHT) != 0);  /* right */

    mm_mark(g_x, g_y, MM_COLOR_BLUE);   /* colour visited cells (debug only) */
}


/* report the whole map in the official format (required after every run).
 * walls = sides we believe are walls, known = sides we really saw;
 * a side not in known is printed as '?' and never costs marks. */
static void dump_map(void){
    mm_map_begin();
    for (int x = 0; x < g_maze.width; x++) {
        for (int y = 0; y < g_maze.height; y++) {
            mm_map_cell(x, y, g_walls[x][y], g_known[x][y]);
        }
    }
    mm_map_end();
}


/* can the mouse leave (x,y) through side dir, according to the map?
 * known side   -> open only if there is no wall
 * unknown side -> unknown_is_open (true when exploring, false for the speed run) */
static bool can_pass(int x, int y, int dir, bool unknown_is_open){
    int nx = x + DX[dir];
    int ny = y + DY[dir];
    uint8_t bit = MM_WALL(dir);

    if (!in_maze(nx, ny))       return false;               /* outside the maze */
    if (g_known[x][y] & bit)    return (g_walls[x][y] & bit) == 0;
    return unknown_is_open;
}

/* true if (x,y) is inside the goal block from mm_maze() (edges included).
 * The goal can be one cell or a block, anywhere, so never hard-code it. */
static bool is_goal(int x, int y){
    return g_maze.goal_x0 <= x && x <= g_maze.goal_x1 &&
           g_maze.goal_y0 <= y && y <= g_maze.goal_y1;
}

/* the open neighbour with the smallest flood distance.
 * Order: straight, right, left, back, so on a tie the mouse
 * keeps going straight and turns as little as possible.
 * Returns -1 if no neighbour can be reached. */
static int best_dir(bool unknown_is_open){
    static const int order[4] = { 0, +1, -1, 2 };   /* straight, right, left, back */
    int best   = -1;
    int best_d = DIST_INF;

    for (int i = 0; i < 4; i++) {
        int d = turn_dir(g_heading, order[i]);              /* absolute direction */
        if (!can_pass(g_x, g_y, d, unknown_is_open)) continue;
        int nd = g_dist[g_x + DX[d]][g_y + DY[d]];          /* neighbour's distance */
        if (nd < best_d) {
            best_d = nd;
            best   = d;
        }
    }
    return best;
}


/* BFS from the target: every cell gets its number of steps to it.
 * to_goal: target = goal block, else the start cell (0,0).
 * unknown_is_open: how to treat sides we have not seen (see can_pass).
 * A queue, not recursion: the ESP32 stack is small, and BFS gives
 * the SHORTEST distance the first time it reaches a cell. */
static void flood(bool to_goal, bool unknown_is_open){
    static int qx[MM_MAX_SIZE * MM_MAX_SIZE];   /* static: 2 KB off the stack */
    static int qy[MM_MAX_SIZE * MM_MAX_SIZE];
    int head = 0, tail = 0;                     /* each cell enters once, 256 is enough */

    for (int x = 0; x < g_maze.width; x++)
        for (int y = 0; y < g_maze.height; y++)
            g_dist[x][y] = DIST_INF;

    if (to_goal) {
        for (int x = 0; x < g_maze.width; x++)
            for (int y = 0; y < g_maze.height; y++)
                if (is_goal(x, y)) {
                    g_dist[x][y] = 0;
                    qx[tail] = x; qy[tail] = y; tail++;
                }
    } else {
        g_dist[0][0] = 0;
        qx[tail] = 0; qy[tail] = 0; tail++;
    }

    while (head < tail) {
        int x = qx[head], y = qy[head];
        head++;
        for (int dir = 0; dir < 4; dir++) {
            if (!can_pass(x, y, dir, unknown_is_open)) continue;
            int nx = x + DX[dir], ny = y + DY[dir];
            if (g_dist[nx][ny] == DIST_INF) {           /* first visit = shortest */
                g_dist[nx][ny] = g_dist[x][y] + 1;
                qx[tail] = nx; qy[tail] = ny; tail++;
            }
        }
    }
}

/* write each cell's distance on the mms maze ("-" = no path).
 * This is what the M1 flood-fill screenshot shows. */
static void show_dist(void){
    char buf[11];                               /* mm_label: max 10 chars */
    for (int x = 0; x < g_maze.width; x++)
        for (int y = 0; y < g_maze.height; y++) {
            if (g_dist[x][y] == DIST_INF) snprintf(buf, sizeof buf, "-");
            else                          snprintf(buf, sizeof buf, "%d", g_dist[x][y]);
            mm_label(x, y, buf);
        }
}

/* the operator pressed Reset: the mouse is back in the start cell.
 * Reset our position (it really moved), but KEEP the map:
 * the walls did not change. Then tell the HAL we are done. */
static void check_reset(void){
    if (!mm_reset_requested()) return;
    g_x = 0;
    g_y = 0;
    g_heading = MM_NORTH;
    mm_log("reset: back to (0,0) facing north, map kept");
    mm_reset_done();
}

/* drive cell by cell to the goal (to_goal = true) or back to the start,
 * sensing and re-flooding at EVERY cell. Unknown sides are treated as
 * open, so the mouse tries the shortest possible path and learns.
 * Returns true if we arrived. */
static bool explore_to(bool to_goal){
    int max_steps = 4 * g_maze.width * g_maze.height;   /* safety limit */

    for (int step = 0; step < max_steps; step++) {
        check_reset();
        sense_and_record();

        bool arrived = to_goal ? is_goal(g_x, g_y)
                               : (g_x == 0 && g_y == 0);
        if (arrived) {
            mm_log("arrived at (%d,%d) after %d steps", g_x, g_y, step);
            return true;
        }

        flood(to_goal, true);          /* recompute with what we know now */
        show_dist();

        if (g_dist[g_x][g_y] == DIST_INF) {
            mm_log("no path from (%d,%d)", g_x, g_y);
            return false;
        }

        int d = best_dir(true);
        if (d < 0) {
            mm_log("stuck at (%d,%d)", g_x, g_y);
            return false;
        }
        if (face(d) != MM_OK) return false;

        mm_status_t st = do_forward(1);
        if (st == MM_BLOCKED) {
            /* the sensors missed this wall: record it, the next
             * flood will route around it */
            set_side(g_x, g_y, g_heading, true);
        } else if (st != MM_OK) {
            return false;              /* MM_FAULT: stop and report */
        }
    }
    mm_log("gave up after %d steps", max_steps);
    return false;
}

/* the race: goal from here by the shortest PROVEN path (unknown sides
 * count as walls, so we can never hit a surprise wall), driving each
 * straight stretch with one mm_forward(n). Falls back to exploring if
 * no proven path exists yet. Returns true if we reached the goal. */
static bool speed_run(void){
    flood(true, false);                        /* only known-open sides */
    show_dist();

    if (g_dist[g_x][g_y] == DIST_INF) {
        mm_log("speed run: no proven path yet, exploring instead");
        return explore_to(true);
    }
    mm_log("speed run: %d cells to the goal", g_dist[g_x][g_y]);

    int max_legs = g_maze.width * g_maze.height;   /* safety limit */
    for (int leg = 0; leg < max_legs && !is_goal(g_x, g_y); leg++) {
        int d = best_dir(false);
        if (d < 0) return explore_to(true);

        /* walk the map (not the mouse) to count how many cells we can
         * go straight in direction d, each one 1 step closer to the goal */
        int n = 0, cx = g_x, cy = g_y;
        while (can_pass(cx, cy, d, false) &&
               g_dist[cx + DX[d]][cy + DY[d]] == g_dist[cx][cy] - 1) {
            cx += DX[d];
            cy += DY[d];
            n++;
            mm_mark(cx, cy, MM_COLOR_GREEN);   /* show the path */
            if (is_goal(cx, cy)) break;
        }

        if (face(d) != MM_OK) return false;
        mm_status_t st = do_forward(n);
        if (st == MM_BLOCKED) {
            /* should never happen on proven sides: a wrong reading.
             * Record it and finish by exploring. */
            set_side(g_x, g_y, g_heading, true);
            return explore_to(true);
        }
        if (st != MM_OK) return false;
    }
    return is_goal(g_x, g_y);
}
//main
void solver_run(void)
{
    mm_begin("T12");

    g_maze = mm_maze();
    g_x = 0;
    g_y = 0;
    g_heading = MM_NORTH;
    init_map();
    mm_log("solver started: maze %dx%d", g_maze.width, g_maze.height);

    /* 1. explore from the start to the goal */
    uint32_t t = mm_millis();
    bool ok = explore_to(true);
    mm_log("RUN 1 explore: %s, %lu ms", ok ? "goal reached" : "FAILED",
           (unsigned long)(mm_millis() - t));
    dump_map();
    if (!ok) return;

    /* 2. come back to the start (explores more on the way) */
    t = mm_millis();
    ok = explore_to(false);
    mm_log("RUN 2 return: %s, %lu ms", ok ? "back at start" : "FAILED",
           (unsigned long)(mm_millis() - t));
    dump_map();
    if (!ok) return;

    /* 3. speed run on the shortest proven path */
    t = mm_millis();
    ok = speed_run();
    mm_log("RUN 3 speed run: %s, %lu ms", ok ? "goal reached" : "FAILED",
           (unsigned long)(mm_millis() - t));
    dump_map();

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
