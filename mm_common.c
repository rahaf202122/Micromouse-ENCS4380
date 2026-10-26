/*
 * mm_common.c - platform-independent part of the ENCS4380 HAL:
 * logging and the map dump. The same file runs in the simulator and on the
 * ESP32, so the dump format is identical on both. DO NOT MODIFY.
 *
 * Dump format (one line each, mixed with any mm_log output):
 *   @MAP <version> team=<id> platform=<sim|esp32> w=<W> h=<H>
 *   @C <x> <y> <N> <E> <S> <W>        each side is 1 (wall), 0 (open), ? (unknown)
 *   @END n=<cells> crc=<hex>
 * crc is CRC-16/CCITT over the text of the @C lines, so edited dumps fail.
 */
#include "mm_internal.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char      g_team[16] = "T??";
static const char *g_platform = "unknown";
static mm_maze_t g_maze;
static int       g_dumping;
static int       g_dump_count;
static uint16_t  g_crc;

static uint16_t crc16_update(uint16_t crc, const char *s)
{
    while (*s) {
        crc ^= (uint16_t)((uint8_t)*s++) << 8;
        for (int i = 0; i < 8; i++)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

void mm__common_begin(const char *team_id, const char *platform, mm_maze_t maze)
{
    char line[128];
    if (team_id && team_id[0]) {
        strncpy(g_team, team_id, sizeof g_team - 1);
        g_team[sizeof g_team - 1] = '\0';
    }
    g_maze = maze;
    if (platform) g_platform = platform;
    snprintf(line, sizeof line,
             "mm: HAL %s, team %s, %s, maze %dx%d, goal (%d,%d)-(%d,%d)",
             MM_HAL_VERSION, g_team, platform, maze.width, maze.height,
             maze.goal_x0, maze.goal_y0, maze.goal_x1, maze.goal_y1);
    mm__write_line(line);
}

void mm_log(const char *fmt, ...)
{
    char line[160];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    /* keep one log call on one line */
    for (char *p = line; *p; p++)
        if (*p == '\n' || *p == '\r') *p = ' ';
    mm__write_line(line);
}

void mm_map_begin(void)
{
    char line[128];
    snprintf(line, sizeof line, "@MAP %s team=%s platform=%s w=%d h=%d",
             MM_HAL_VERSION, g_team, g_platform, g_maze.width, g_maze.height);
    mm__write_line(line);
    g_dumping = 1;
    g_dump_count = 0;
    g_crc = 0xFFFF;
}

static char side(uint8_t walls, uint8_t known, int dir)
{
    if (!(known & MM_WALL(dir))) return '?';
    return (walls & MM_WALL(dir)) ? '1' : '0';
}

void mm_map_cell(int x, int y, uint8_t walls, uint8_t known)
{
    char line[48];
    if (!g_dumping) {
        mm__write_line("mm: error: mm_map_cell() called before mm_map_begin()");
        return;
    }
    if (x < 0 || y < 0 || x >= g_maze.width || y >= g_maze.height) {
        mm_log("mm: error: mm_map_cell(%d,%d) is outside the maze", x, y);
        return;
    }
    snprintf(line, sizeof line, "@C %d %d %c %c %c %c", x, y,
             side(walls, known, MM_NORTH), side(walls, known, MM_EAST),
             side(walls, known, MM_SOUTH), side(walls, known, MM_WEST));
    g_crc = crc16_update(g_crc, line);
    g_dump_count++;
    mm__write_line(line);
}

void mm_map_end(void)
{
    char line[48];
    if (!g_dumping) {
        mm__write_line("mm: error: mm_map_end() called before mm_map_begin()");
        return;
    }
    snprintf(line, sizeof line, "@END n=%d crc=%04X", g_dump_count, (unsigned)g_crc);
    mm__write_line(line);
    g_dumping = 0;
}
