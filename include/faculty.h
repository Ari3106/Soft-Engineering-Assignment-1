#ifndef FACULTY_H
#define FACULTY_H

#include "course.h"

#define MAX_FACULTY 100
#define DEPT_LEN    48

typedef struct {
    char name[NAME_LEN];
    char department[DEPT_LEN];
    int weekly_office_hours;      /* required office hours per week, in whole hours */
    char office_room[ROOM_LEN];
    unsigned char preferred_days; /* DAY_* bitmask, used as fallback when teaching
                                      days alone can't cover the required hours */
} Faculty;

typedef struct {
    Faculty faculty[MAX_FACULTY];
    int count;
} FacultyList;

/* Returns a pointer to the faculty member named `name`, or NULL if not found. */
const Faculty *find_faculty_by_name(const FacultyList *list, const char *name);

#endif /* FACULTY_H */
