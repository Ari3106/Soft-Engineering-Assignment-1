#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include "course.h"
#include "faculty.h"

#define MAX_OFFICE_BLOCKS 20

typedef struct {
    int day_index;              /* 0=Mon .. 4=Fri */
    int start_min;
    int end_min;
    char related_course[CODE_LEN]; /* course this block is adjacent to, or "" for a
                                       general (non-class-adjacent) block */
} OfficeBlock;

typedef struct {
    char instructor[NAME_LEN];
    OfficeBlock blocks[MAX_OFFICE_BLOCKS];
    int block_count;
    int minutes_assigned;
    int minutes_required;
} OfficeSchedule;

typedef struct {
    OfficeSchedule schedules[MAX_FACULTY];
    int count;
} OfficeScheduleList;

/* Builds one office-hour schedule per faculty member. Office hours are placed
   in the gaps around each instructor's own class times (never overlapping a
   class), and only fall back to a faculty member's preferred days if their
   teaching days don't leave enough free time to satisfy weekly_office_hours. */
void generate_office_schedules(const CourseList *courses,
                                const FacultyList *faculty,
                                OfficeScheduleList *out);

/* Writes the combined teaching + office-hours report (the human-readable
   output, in the style of the sample office-hours table) to `filename`. */
int write_report(const char *filename,
                  const CourseList *courses,
                  const FacultyList *faculty,
                  const OfficeScheduleList *office_list);

/* Writes a flat CSV of the office-hours blocks to `filename`, suitable for
   the Python script to turn into a Word/Excel document. */
int write_csv_summary(const char *filename,
                       const CourseList *courses,
                       const OfficeScheduleList *office_list);

/* Writes a flat CSV of every course section (one row per section, with its
   own instructor) to `filename`, so the Python script can render each
   instructor's "CLASS SCHEDULE" table alongside their office hours. */
int write_csv_courses(const char *filename, const CourseList *courses);

#endif /* SCHEDULER_H */
