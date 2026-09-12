#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

#define WORKDAY_START (8 * 60)   /* 8:00 AM - office hours window opens */
#define WORKDAY_END   (17 * 60)  /* 5:00 PM - office hours window closes */
#define DEFAULT_BLOCK 120        /* preferred office-hour block: 2 hours   */
#define MIN_BLOCK     30         /* shortest block worth scheduling        */

static int cmp_start(const void *a, const void *b) {
    const Course *ca = *(const Course * const *)a;
    const Course *cb = *(const Course * const *)b;
    return ca->start_min - cb->start_min;
}

/* Collects the courses `name` teaches on `day_index`, sorted by start time. */
static int collect_day_courses(const CourseList *courses, const char *name,
                                int day_index, const Course *out[]) {
    int n = 0;
    for (int i = 0; i < courses->count; i++) {
        const Course *c = &courses->courses[i];
        if (strcmp(c->instructor, name) == 0 && course_meets_on(c, day_index)) {
            out[n++] = c;
        }
    }
    qsort(out, n, sizeof(Course *), cmp_start);
    return n;
}

static void add_block(OfficeSchedule *sched, int day, int start, int end, const char *course_code) {
    if (sched->block_count >= MAX_OFFICE_BLOCKS) return;
    OfficeBlock *b = &sched->blocks[sched->block_count++];
    b->day_index = day;
    b->start_min = start;
    b->end_min = end;
    strncpy(b->related_course, course_code, CODE_LEN - 1);
    b->related_course[CODE_LEN - 1] = '\0';
    sched->minutes_assigned += (end - start);
}

static int min_int(int a, int b) { return a < b ? a : b; }

/* Fills the gaps around a single faculty member's classes with office-hour
   blocks until `remaining` minutes have been placed or the week runs out. */
static void schedule_around_classes(const CourseList *courses, OfficeSchedule *sched, int *remaining) {
    for (int day = 0; day < NUM_DAYS && *remaining > 0; day++) {
        const Course *day_courses[MAX_COURSES];
        int n = collect_day_courses(courses, sched->instructor, day, day_courses);
        if (n == 0) continue;

        /* Gap before the first class of the day: hold office hours right
           before it starts (this is what covers evening-only instructors,
           whose only free lab hours are in the morning). */
        if (*remaining > 0) {
            int gap_start = WORKDAY_START;
            int gap_end = day_courses[0]->start_min;
            int available = gap_end - gap_start;
            if (available >= MIN_BLOCK) {
                int len = min_int(available, min_int(DEFAULT_BLOCK, *remaining));
                add_block(sched, day, gap_end - len, gap_end, day_courses[0]->code);
                *remaining -= len;
            }
        }

        for (int i = 0; i < n && *remaining > 0; i++) {
            int gap_start = day_courses[i]->end_min;
            int gap_end = (i + 1 < n) ? day_courses[i + 1]->start_min : WORKDAY_END;
            int available = gap_end - gap_start;
            if (available < MIN_BLOCK) continue;

            if (available >= 2 * DEFAULT_BLOCK && i + 1 < n) {
                /* Big gap between two classes: hold office hours right after
                   the class that just ended and, if hours remain, right
                   before the next one begins. */
                int len1 = min_int(DEFAULT_BLOCK, *remaining);
                add_block(sched, day, gap_start, gap_start + len1, day_courses[i]->code);
                *remaining -= len1;

                if (*remaining > 0) {
                    int len2 = min_int(DEFAULT_BLOCK, *remaining);
                    int start2 = gap_end - len2;
                    if (start2 < gap_start + len1) start2 = gap_start + len1;
                    len2 = gap_end - start2;
                    if (len2 >= MIN_BLOCK) {
                        add_block(sched, day, start2, gap_end, day_courses[i + 1]->code);
                        *remaining -= len2;
                    }
                }
            } else {
                int len = min_int(available, min_int(DEFAULT_BLOCK, *remaining));
                add_block(sched, day, gap_start, gap_start + len, day_courses[i]->code);
                *remaining -= len;
            }
        }
    }
}

/* Used once teaching-day gaps can't absorb all the required hours: falls
   back to a faculty member's stated preferred days with a plain morning
   block, skipping any day they already teach on. */
static void schedule_fallback_days(const CourseList *courses, const Faculty *fac,
                                    OfficeSchedule *sched, int *remaining) {
    for (int day = 0; day < NUM_DAYS && *remaining > 0; day++) {
        static const unsigned char bits[NUM_DAYS] = { DAY_MON, DAY_TUE, DAY_WED, DAY_THU, DAY_FRI };
        if (!(fac->preferred_days & bits[day])) continue;

        const Course *day_courses[MAX_COURSES];
        int n = collect_day_courses(courses, sched->instructor, day, day_courses);
        if (n > 0) continue; /* already handled by schedule_around_classes */

        int len = min_int(DEFAULT_BLOCK, *remaining);
        add_block(sched, day, WORKDAY_START, WORKDAY_START + len, "");
        *remaining -= len;
    }
}

void generate_office_schedules(const CourseList *courses, const FacultyList *faculty, OfficeScheduleList *out) {
    out->count = 0;
    for (int i = 0; i < faculty->count; i++) {
        const Faculty *fac = &faculty->faculty[i];
        OfficeSchedule *sched = &out->schedules[out->count];
        memset(sched, 0, sizeof(*sched));
        strncpy(sched->instructor, fac->name, NAME_LEN - 1);
        sched->instructor[NAME_LEN - 1] = '\0';
        sched->minutes_required = fac->weekly_office_hours * 60;

        int remaining = sched->minutes_required;
        schedule_around_classes(courses, sched, &remaining);
        schedule_fallback_days(courses, fac, sched, &remaining);

        out->count++;
    }
}

static void fmt_range(char *buf, size_t size, int start, int end) {
    char s[16], e[16];
    minutes_to_time_str(start, s, sizeof(s));
    minutes_to_time_str(end, e, sizeof(e));
    snprintf(buf, size, "%s - %s", s, e);
}

static void print_teaching_table(FILE *f, const CourseList *courses, const char *instructor) {
    fprintf(f, "  %-10s %-38s %-18s %-8s\n", "Course", "Title", "Times", "Room");
    fprintf(f, "  %-10s %-38s %-18s %-8s\n", "------", "-----", "-----", "----");
    for (int i = 0; i < courses->count; i++) {
        const Course *c = &courses->courses[i];
        if (strcmp(c->instructor, instructor) != 0) continue;

        char days[8] = { 0 };
        int p = 0;
        for (int d = 0; d < NUM_DAYS; d++) {
            if (course_meets_on(c, d)) days[p++] = day_letter(d);
        }
        days[p] = '\0';

        char range[24];
        fmt_range(range, sizeof(range), c->start_min, c->end_min);

        char times[40];
        snprintf(times, sizeof(times), "%s %s", days, range);

        fprintf(f, "  %-10s %-38s %-18s %-8s\n", c->code, c->title, times, c->room);
    }
}

static void print_office_hours(FILE *f, const OfficeSchedule *sched) {
    fprintf(f, "  OFFICE HOURS  (%d of %d required hours/week scheduled)\n",
            sched->minutes_assigned / 60, sched->minutes_required / 60);
    fprintf(f, "  NOTE: Instructors are not to be interrupted during class times.\n\n");

    for (int day = 0; day < NUM_DAYS; day++) {
        fprintf(f, "    %-10s", day_name(day));
        int printed = 0;
        for (int i = 0; i < sched->block_count; i++) {
            const OfficeBlock *b = &sched->blocks[i];
            if (b->day_index != day) continue;

            char range[24];
            fmt_range(range, sizeof(range), b->start_min, b->end_min);

            if (printed) fprintf(f, "\n    %-10s&  %s", "", range);
            else fprintf(f, "  %s", range);

            if (b->related_course[0] != '\0') fprintf(f, " (%s)", b->related_course);
            printed = 1;
        }
        if (!printed) fprintf(f, "  --");
        fprintf(f, "\n");
    }
}

int write_report(const char *filename, const CourseList *courses, const FacultyList *faculty,
                  const OfficeScheduleList *office_list) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: could not open '%s' for writing\n", filename);
        return -1;
    }

    fprintf(f, "================================================================\n");
    fprintf(f, "           FACULTY TEACHING & OFFICE HOURS SCHEDULE\n");
    fprintf(f, "================================================================\n\n");

    for (int i = 0; i < faculty->count; i++) {
        const Faculty *fac = &faculty->faculty[i];
        fprintf(f, "----------------------------------------------------------------\n");
        fprintf(f, "%s  (%s)\n", fac->name, fac->department);
        fprintf(f, "Office: %s\n", fac->office_room);
        fprintf(f, "----------------------------------------------------------------\n\n");

        fprintf(f, "CLASS SCHEDULE\n");
        print_teaching_table(f, courses, fac->name);
        fprintf(f, "\n");

        for (int j = 0; j < office_list->count; j++) {
            if (strcmp(office_list->schedules[j].instructor, fac->name) == 0) {
                print_office_hours(f, &office_list->schedules[j]);
                break;
            }
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return 0;
}

int write_csv_courses(const char *filename, const CourseList *courses) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: could not open '%s' for writing\n", filename);
        return -1;
    }

    fprintf(f, "Instructor,Code,Title,Days,Start,End,Room\n");
    for (int i = 0; i < courses->count; i++) {
        const Course *c = &courses->courses[i];

        char days[8] = { 0 };
        int p = 0;
        for (int d = 0; d < NUM_DAYS; d++) {
            if (course_meets_on(c, d)) days[p++] = day_letter(d);
        }
        days[p] = '\0';

        char s[16], e[16];
        minutes_to_time_str(c->start_min, s, sizeof(s));
        minutes_to_time_str(c->end_min, e, sizeof(e));

        fprintf(f, "\"%s\",\"%s\",\"%s\",%s,%s,%s,\"%s\"\n",
                c->instructor, c->code, c->title, days, s, e, c->room);
    }

    fclose(f);
    return 0;
}

int write_csv_summary(const char *filename, const CourseList *courses, const OfficeScheduleList *office_list) {
    (void)courses;
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: could not open '%s' for writing\n", filename);
        return -1;
    }

    fprintf(f, "Instructor,Day,Start,End,RelatedCourse\n");
    for (int i = 0; i < office_list->count; i++) {
        const OfficeSchedule *sched = &office_list->schedules[i];
        for (int j = 0; j < sched->block_count; j++) {
            const OfficeBlock *b = &sched->blocks[j];
            char s[16], e[16];
            minutes_to_time_str(b->start_min, s, sizeof(s));
            minutes_to_time_str(b->end_min, e, sizeof(e));
            fprintf(f, "\"%s\",%s,%s,%s,%s\n",
                    sched->instructor, day_name(b->day_index), s, e, b->related_course);
        }
    }

    fclose(f);
    return 0;
}
