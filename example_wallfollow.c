/*
 * example_wallfollow.c - shows how to use the HAL. NOT a solution.
 *
 * A left-hand wall follower, the same idea as the mms-c example, rewritten
 * against this semester's HAL. It never finds the goal in a competition
 * maze, and it does not track position or build a map: that is your job.
 *
 * Build it instead of solver.c:   make example   then run ./example
 */
#include "mouse_hal.h"

void solver_run(void)
{
    mm_begin("EXAMPLE");
    mm_log("left-hand wall follower: this will not find the goal");

    for (int step = 0; step < 500; step++) {
        uint8_t s = mm_sense();

        if (!(s & MM_SEE_LEFT))       mm_turn(-1);   /* open on the left: go left */
        else if (!(s & MM_SEE_FRONT)) { /* go straight */ }
        else if (!(s & MM_SEE_RIGHT)) mm_turn(+1);
        else                          mm_turn(2);    /* dead end */

        if (mm_forward(1) != MM_OK) {
            mm_log("blocked at step %d", step);
            return;
        }
    }
}
