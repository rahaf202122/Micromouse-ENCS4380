/*
 * hal_esp32.cpp - ENCS4380 HAL implementation for YOUR robot.
 * Team T?? - robot name: ????
 *
 * This is a skeleton. You write the functions marked TODO, milestone by
 * milestone. Keep the function names and behaviour exactly as described in
 * mouse_hal.h: your solver.c must run here without any change.
 *
 * The parts already written (begin, maze, logging, time, radios) are the
 * same for every team. Everything that moves the robot or reads a sensor is
 * yours.
 */
#include <Arduino.h>
#include <WiFi.h>
#include "mm_internal.h"
#include "robot_config.h"

static uint32_t g_t0;

/* ---- given: logging, set-up, maze -------------------------------------- */

extern "C" void mm__write_line(const char *line)
{
    Serial.println(line);
}

extern "C" void mm_begin(const char *team_id)
{
    Serial.begin(115200);
    delay(200);

#if !MM_RADIOS_ENABLED
    WiFi.mode(WIFI_OFF);     /* competition rule 1: no outside signals */
    btStop();
#endif

    /* TODO M3: pins, I2C, ToF address assignment (XSHUT), motor driver,
     *          buttons, LED. Keep each part in its own small function. */
    /* TODO M4: encoder counting (interrupts first, then PCNT). */

    mm_maze_t m = mm_maze();
    g_t0 = millis();
    mm__common_begin(team_id, "esp32", m);
}

extern "C" mm_maze_t mm_maze(void)
{
    mm_maze_t m;
    m.width   = MAZE_WIDTH;   m.height  = MAZE_HEIGHT;
    m.goal_x0 = GOAL_X0;      m.goal_y0 = GOAL_Y0;
    m.goal_x1 = GOAL_X1;      m.goal_y1 = GOAL_Y1;
    return m;
}

extern "C" uint32_t mm_millis(void)
{
    return millis() - g_t0;
}

/* ---- yours: sensing ------------------------------------------------------ */

extern "C" uint8_t mm_sense(void)
{
    /* TODO M7: read the side and front ToF sensors at the wall-reading
     * point, average a few readings, compare with WALL_*_MM, and return
     * MM_SEE_LEFT / MM_SEE_FRONT / MM_SEE_RIGHT bits. */
    return MM_SEE_LEFT | MM_SEE_FRONT | MM_SEE_RIGHT;   /* placeholder */
}

/* ---- yours: motion ------------------------------------------------------- */

extern "C" mm_status_t mm_forward(int cells)
{
    /* TODO M4: drive one cell by encoder count and stop.
     * TODO M5: straight-line control, wall centring, speed profile, and
     *          'cells' greater than 1 without stopping in between.
     * Return MM_BLOCKED (without hitting it) if a wall is ahead.
     * End at the centre of the last cell. */
    (void)cells;
    return MM_FAULT;   /* placeholder: not written yet */
}

extern "C" mm_status_t mm_turn(int quarter_turns)
{
    /* TODO M6: turn in place by encoder count (90 deg = pi * TRACK_WIDTH_MM / 4
     * per wheel), then square up on a front wall when there is one. */
    (void)quarter_turns;
    return MM_FAULT;   /* placeholder: not written yet */
}

/* ---- yours: restart button ----------------------------------------------- */

extern "C" bool mm_reset_requested(void)
{
    /* TODO M7: return true when the operator presses the reset button
     * after putting the robot back in the start cell. */
    return false;
}

extern "C" void mm_reset_done(void)
{
    /* TODO M7: wait for the start button, then return. */
}

/* ---- optional on the robot ----------------------------------------------- */

extern "C" void mm_mark(int x, int y, char color)       { (void)x; (void)y; (void)color; }
extern "C" void mm_label(int x, int y, const char *t)   { (void)x; (void)y; (void)t; }
extern "C" void mm_show_wall(int x, int y, int dir)     { (void)x; (void)y; (void)dir; }
extern "C" void mm_clear_marks(void)                    { }
