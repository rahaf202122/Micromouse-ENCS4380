/*
 * micromouse.ino - ESP32 sketch for the ENCS4380 micromouse.
 *
 * This folder must contain (run "make sync" in the parent folder to copy
 * the shared files here):
 *   mouse_hal.h  mm_internal.h  mm_common.c  solver.c      (shared, unchanged)
 *   hal_esp32.cpp  robot_config.h                          (yours)
 *
 * During M3-M6, write small test sketches for each subsystem instead of
 * running the solver. From M7 the robot runs solver_run() below.
 */
#include <Arduino.h>
#include "mouse_hal.h"
#include "robot_config.h"

void setup()
{
    /* TODO M7: wait here until the start button is pressed. */
    solver_run();
}

void loop()
{
    delay(1000);   /* the solver has finished */
}
