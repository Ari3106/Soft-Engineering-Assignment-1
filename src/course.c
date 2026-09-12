#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "course.h"

unsigned char parse_days(const char *daystr) {
    unsigned char mask = 0;
    for (const char *p = daystr; *p != '\0'; p++) {
        switch (toupper((unsigned char)*p)) {
            case 'M': mask |= DAY_MON; break;
            case 'T': mask |= DAY_TUE; break;
            case 'W': mask |= DAY_WED; break;
            case 'R': mask |= DAY_THU; break;
            case 'F': mask |= DAY_FRI; break;
            default: break; /* ignore separators/whitespace */
        }
    }
    return mask;
}

const char *day_name(int day_index) {
    static const char *names[NUM_DAYS] = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
    };
    if (day_index < 0 || day_index >= NUM_DAYS) return "?";
    return names[day_index];
}

char day_letter(int day_index) {
    static const char letters[NUM_DAYS] = { 'M', 'T', 'W', 'R', 'F' };
    if (day_index < 0 || day_index >= NUM_DAYS) return '?';
    return letters[day_index];
}

int course_meets_on(const Course *c, int day_index) {
    static const unsigned char bits[NUM_DAYS] = {
        DAY_MON, DAY_TUE, DAY_WED, DAY_THU, DAY_FRI
    };
    if (day_index < 0 || day_index >= NUM_DAYS) return 0;
    return (c->days & bits[day_index]) != 0;
}

int time_to_minutes(const char *hhmm) {
    int h, m;
    if (sscanf(hhmm, "%d:%d", &h, &m) != 2) return -1;
    if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
    return h * 60 + m;
}

void minutes_to_time_str(int minutes, char *out, size_t outsize) {
    int h24 = (minutes / 60) % 24;
    int m = minutes % 60;
    int h12 = h24 % 12;
    if (h12 == 0) h12 = 12;
    const char *ampm = (h24 < 12) ? "AM" : "PM";
    snprintf(out, outsize, "%d:%02d %s", h12, m, ampm);
}
