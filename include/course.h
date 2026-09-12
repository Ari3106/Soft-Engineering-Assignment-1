#ifndef COURSE_H
#define COURSE_H

#include <stddef.h>

#define MAX_COURSES 200
#define CODE_LEN   16
#define TITLE_LEN  64
#define NAME_LEN   48
#define ROOM_LEN   16

/* Bitmask for days of the week (Mon-Fri only; the lab schedules no weekend classes). */
#define DAY_MON 0x01
#define DAY_TUE 0x02
#define DAY_WED 0x04
#define DAY_THU 0x08
#define DAY_FRI 0x10
#define NUM_DAYS 5

typedef struct {
    char code[CODE_LEN];
    char title[TITLE_LEN];
    char instructor[NAME_LEN];
    unsigned char days;   /* bitmask of DAY_* */
    int start_min;        /* minutes since midnight */
    int end_min;
    char room[ROOM_LEN];
} Course;

typedef struct {
    Course courses[MAX_COURSES];
    int count;
} CourseList;

/* Parses a day string such as "MW", "TR", "MWF" into a DAY_* bitmask. */
unsigned char parse_days(const char *daystr);

/* Returns the full weekday name for index 0=Mon .. 4=Fri. */
const char *day_name(int day_index);

/* Returns the single-letter code (M/T/W/R/F) for index 0=Mon .. 4=Fri. */
char day_letter(int day_index);

/* True if the course meets on the given day index (0=Mon .. 4=Fri). */
int course_meets_on(const Course *c, int day_index);

/* Converts "HH:MM" (24-hour) into minutes since midnight. Returns -1 on error. */
int time_to_minutes(const char *hhmm);

/* Formats minutes-since-midnight as "h:MM AM/PM" into out (outsize must be >= 9). */
void minutes_to_time_str(int minutes, char *out, size_t outsize);

#endif /* COURSE_H */
