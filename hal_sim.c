/*
 * hal_sim.c - ENCS4380 HAL implementation for the mms simulator.
 * Talks to mms over stdin/stdout using the mms mouse API
 * (https://github.com/mackorone/mms#mouse-api). DO NOT MODIFY.
 *
 * Goal block: mms always shows the centre as the goal, but hidden test mazes
 * may use another goal. The goal is taken from, in order:
 *   1. environment variable MM_GOAL="x0,y0,x1,y1" (or "x,y")
 *   2. a file mm_goal.txt in the run directory with the same text
 *   3. the centre of the maze (2x2 for even sizes, 1x1 for odd)
 */
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 199309L   /* clock_gettime with -std=c99 */
#endif
#include "mm_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static uint32_t now_ms(void) { return (uint32_t)GetTickCount(); }
#else
#include <time.h>
static uint32_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}
#endif

static mm_maze_t g_maze;
static int       g_ready;
static uint32_t  g_t0;

/* ---- protocol ---------------------------------------------------------- */

static void send_cmd(const char *cmd)
{
    fputs(cmd, stdout);
    fputc('\n', stdout);
    fflush(stdout);
}

static void read_reply(char *buf, int n)
{
    if (!fgets(buf, n, stdin)) {
        fputs("mm: the simulator closed the connection\n", stderr);
        exit(1);
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

static int ask_int(const char *cmd)
{
    char buf[32];
    send_cmd(cmd);
    read_reply(buf, sizeof buf);
    return atoi(buf);
}

static bool ask_bool(const char *cmd)
{
    char buf[32];
    send_cmd(cmd);
    read_reply(buf, sizeof buf);
    return strcmp(buf, "true") == 0;
}

static bool ask_ack(const char *cmd)   /* true = "ack", false = "crash" */
{
    char buf[32];
    send_cmd(cmd);
    read_reply(buf, sizeof buf);
    return strcmp(buf, "ack") == 0;
}

/* ---- goal -------------------------------------------------------------- */

static int parse_goal(const char *s, mm_maze_t *m)
{
    int a, b, c, d;
    int n = sscanf(s, " %d , %d , %d , %d", &a, &b, &c, &d);
    if (n == 2) { c = a; d = b; }
    else if (n != 4) return 0;
    if (a > c) { int t = a; a = c; c = t; }
    if (b > d) { int t = b; b = d; d = t; }
    if (a < 0 || b < 0 || c >= m->width || d >= m->height) return 0;
    m->goal_x0 = a; m->goal_y0 = b; m->goal_x1 = c; m->goal_y1 = d;
    return 1;
}

static void set_goal(mm_maze_t *m)
{
    char buf[64];
    const char *env = getenv("MM_GOAL");
    FILE *f;

    if (env && parse_goal(env, m)) return;
    f = fopen("mm_goal.txt", "r");
    if (f) {
        int ok = fgets(buf, sizeof buf, f) && parse_goal(buf, m);
        fclose(f);
        if (ok) return;
        fputs("mm: warning: mm_goal.txt is not valid, using the centre\n", stderr);
    }
    m->goal_x0 = (m->width - 1) / 2;  m->goal_x1 = m->width / 2;
    m->goal_y0 = (m->height - 1) / 2; m->goal_y1 = m->height / 2;
}

static void init_once(void)
{
    if (g_ready) return;
    g_maze.width  = ask_int("mazeWidth");
    g_maze.height = ask_int("mazeHeight");
    if (g_maze.width < 1 || g_maze.height < 1 ||
        g_maze.width > MM_MAX_SIZE || g_maze.height > MM_MAX_SIZE) {
        fprintf(stderr, "mm: unsupported maze size %dx%d\n", g_maze.width, g_maze.height);
        exit(1);
    }
    set_goal(&g_maze);
    g_t0 = now_ms();
    g_ready = 1;
}

/* ---- HAL API ----------------------------------------------------------- */

void mm__write_line(const char *line)
{
    fputs(line, stderr);
    fputc('\n', stderr);
    fflush(stderr);
}

void mm_begin(const char *team_id)
{
    init_once();
    mm__common_begin(team_id, "sim", g_maze);
}

mm_maze_t mm_maze(void)
{
    init_once();
    return g_maze;
}

uint8_t mm_sense(void)
{
    uint8_t s = 0;
    if (ask_bool("wallLeft"))  s |= MM_SEE_LEFT;
    if (ask_bool("wallFront")) s |= MM_SEE_FRONT;
    if (ask_bool("wallRight")) s |= MM_SEE_RIGHT;
    return s;
}

mm_status_t mm_forward(int cells)
{
    char cmd[32];
    if (cells == 0) return MM_OK;
    if (cells < 0 || cells > MM_MAX_SIZE) return MM_FAULT;
#ifdef MM_SIM_SINGLE_STEPS
    /* for old mms builds without "moveForward N": one cell per command */
    for (int i = 0; i < cells; i++)
        if (!ask_ack("moveForward")) return MM_BLOCKED;
    (void)cmd;
    return MM_OK;
#else
    snprintf(cmd, sizeof cmd, "moveForward %d", cells);
    return ask_ack(cmd) ? MM_OK : MM_BLOCKED;
#endif
}

mm_status_t mm_turn(int quarter_turns)
{
    int q = ((quarter_turns % 4) + 4) % 4;
    switch (q) {
    case 1:  return ask_ack("turnRight") ? MM_OK : MM_FAULT;
    case 2:  if (!ask_ack("turnRight")) return MM_FAULT;
             return ask_ack("turnRight") ? MM_OK : MM_FAULT;
    case 3:  return ask_ack("turnLeft") ? MM_OK : MM_FAULT;
    default: return MM_OK;
    }
}

bool mm_reset_requested(void) { return ask_bool("wasReset"); }
void mm_reset_done(void)      { ask_ack("ackReset"); }

static int in_maze(int x, int y)
{
    return g_ready && x >= 0 && y >= 0 && x < g_maze.width && y < g_maze.height;
}

void mm_mark(int x, int y, char color)
{
    char cmd[40];
    if (!in_maze(x, y)) return;
    if (color == MM_COLOR_NONE) snprintf(cmd, sizeof cmd, "clearColor %d %d", x, y);
    else                        snprintf(cmd, sizeof cmd, "setColor %d %d %c", x, y, color);
    send_cmd(cmd);
}

void mm_label(int x, int y, const char *text)
{
    char cmd[48], t[11];
    int n = 0;
    if (!in_maze(x, y)) return;
    if (!text || !text[0]) {
        snprintf(cmd, sizeof cmd, "clearText %d %d", x, y);
        send_cmd(cmd);
        return;
    }
    for (; text[n] && n < 10; n++)   /* printable, no spaces, max 10 chars */
        t[n] = (text[n] > ' ' && text[n] < 127) ? text[n] : '_';
    t[n] = '\0';
    snprintf(cmd, sizeof cmd, "setText %d %d %s", x, y, t);
    send_cmd(cmd);
}

void mm_show_wall(int x, int y, int dir)
{
    static const char names[4] = { 'n', 'e', 's', 'w' };
    char cmd[40];
    if (!in_maze(x, y)) return;
    snprintf(cmd, sizeof cmd, "setWall %d %d %c", x, y, names[dir & 3]);
    send_cmd(cmd);
}

void mm_clear_marks(void)
{
    send_cmd("clearAllColor");
    send_cmd("clearAllText");
}

uint32_t mm_millis(void) { return now_ms() - g_t0; }
