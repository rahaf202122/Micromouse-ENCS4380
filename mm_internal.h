/*
 * mm_internal.h - shared between mm_common.c and the platform HALs.
 * Not for use in solver.c. DO NOT MODIFY.
 */
#ifndef MM_INTERNAL_H
#define MM_INTERNAL_H

#include "mouse_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Implemented by each platform HAL (hal_sim.c, hal_esp32.cpp):
 * write one complete line of text (no newline included) to the debug
 * channel: stderr in the simulator, Serial on the robot. */
void mm__write_line(const char *line);

/* Implemented in mm_common.c; each platform's mm_begin() calls it. */
void mm__common_begin(const char *team_id, const char *platform, mm_maze_t maze);

#ifdef __cplusplus
}
#endif

#endif /* MM_INTERNAL_H */
