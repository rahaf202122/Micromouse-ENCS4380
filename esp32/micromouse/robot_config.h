/*
 * robot_config.h - YOUR robot's settings. Edit this file.
 * Everything that depends on your hardware or calibration lives here, so
 * hal_esp32.cpp and solver.c never contain magic numbers.
 */
#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#define TEAM_ID          "T00"     /* same as in solver.c */

/* ---- Maze (set before check-in; no changes after the maze is shown) ---- */
#define MAZE_WIDTH       8
#define MAZE_HEIGHT      8
#define GOAL_X0          3         /* goal block, inclusive cell coordinates */
#define GOAL_Y0          3
#define GOAL_X1          4
#define GOAL_Y1          4

/* ---- Competition rule 1: no radio signals during a run ---------------- */
/* 0 = Wi-Fi and Bluetooth switched off in mm_begin() (required on race day)
 * 1 = radios left on for debugging during development */
#define MM_RADIOS_ENABLED 0

/* ---- Pins (fill in from your schematic, M3) ---------------------------- */
#define PIN_BUTTON_START  -1       /* TODO M3 */
#define PIN_BUTTON_RESET  -1       /* TODO M3 */
#define PIN_LED           -1       /* TODO M3 */
#define PIN_BATTERY_ADC   -1       /* TODO M3: an ADC1 pin (classic ESP32: GPIO 32-39) */

#define PIN_I2C_SDA       -1       /* TODO M3 */
#define PIN_I2C_SCL       -1       /* TODO M3 */
/* one XSHUT pin per ToF sensor, in this order: left, front-left, front,
 * front-right, right (use -1 for a sensor you do not fit) */
#define PIN_TOF_XSHUT     { -1, -1, -1, -1, -1 }   /* TODO M3 */

#define PIN_MOTOR_L_IN1   -1       /* TODO M3 */
#define PIN_MOTOR_L_IN2   -1
#define PIN_MOTOR_L_PWM   -1
#define PIN_MOTOR_R_IN1   -1
#define PIN_MOTOR_R_IN2   -1
#define PIN_MOTOR_R_PWM   -1
#define PIN_MOTOR_STBY    -1

#define PIN_ENC_L_A       -1       /* TODO M4 */
#define PIN_ENC_L_B       -1
#define PIN_ENC_R_A       -1
#define PIN_ENC_R_B       -1

/* ---- Calibration (measured on YOUR robot) ------------------------------ */
#define CELL_MM              180.0f   /* centre to centre */
#define TICKS_PER_MM_LEFT    0.0f     /* TODO M4: from the 1 m ruler test   */
#define TICKS_PER_MM_RIGHT   0.0f     /* TODO M4                            */
#define TRACK_WIDTH_MM       0.0f     /* TODO M6: from the ten-spin test     */

#define WALL_LEFT_MM         0        /* TODO M7: from your threshold data  */
#define WALL_FRONT_MM        0        /* TODO M7                            */
#define WALL_RIGHT_MM        0        /* TODO M7                            */

#endif /* ROBOT_CONFIG_H */
