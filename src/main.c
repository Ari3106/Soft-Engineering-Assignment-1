/*
 * COSC 369 - Software Engineering I - Project 1
 *
 * Reads a class-schedule file and a faculty-information file, then generates
 * each instructor's teaching schedule and a conflict-free office-hours
 * schedule that satisfies their required weekly office hours.
 *
 * Usage: scheduler <class_schedule_file> <faculty_info_file> [output_dir]
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "course.h"
#include "faculty.h"
#include "parser.h"
#include "scheduler.h"

static void ensure_dir(const char *path) {
    mkdir(path, 0755); /* ignore EEXIST and other errors; write calls below will report failures */
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <class_schedule_file> <faculty_info_file> [output_dir]\n", argv[0]);
        return 1;
    }

    const char *class_file = argv[1];
    const char *faculty_file = argv[2];
    const char *out_dir = (argc == 4) ? argv[3] : "output";

    static CourseList courses;
    static FacultyList faculty;
    static OfficeScheduleList office_list;

    if (load_course_list(class_file, &courses) != 0) return 1;
    if (load_faculty_list(faculty_file, &faculty) != 0) return 1;

    printf("Loaded %d course(s) from '%s'.\n", courses.count, class_file);
    printf("Loaded %d faculty member(s) from '%s'.\n", faculty.count, faculty_file);

    /* Flag courses taught by someone missing from the faculty file: office
       hours can't be generated for them since we have no weekly-hours policy. */
    for (int i = 0; i < courses.count; i++) {
        if (find_faculty_by_name(&faculty, courses.courses[i].instructor) == NULL) {
            fprintf(stderr, "Warning: '%s' teaches %s but has no entry in the faculty file\n",
                    courses.courses[i].instructor, courses.courses[i].code);
        }
    }

    generate_office_schedules(&courses, &faculty, &office_list);

    for (int i = 0; i < office_list.count; i++) {
        const OfficeSchedule *s = &office_list.schedules[i];
        if (s->minutes_assigned < s->minutes_required) {
            fprintf(stderr, "Warning: only scheduled %d of %d required office-hour minutes for %s\n",
                    s->minutes_assigned, s->minutes_required, s->instructor);
        }
    }

    ensure_dir(out_dir);

    char report_path[512], office_csv_path[512], courses_csv_path[512];
    snprintf(report_path, sizeof(report_path), "%s/schedule_report.txt", out_dir);
    snprintf(office_csv_path, sizeof(office_csv_path), "%s/office_hours.csv", out_dir);
    snprintf(courses_csv_path, sizeof(courses_csv_path), "%s/class_schedule.csv", out_dir);

    if (write_report(report_path, &courses, &faculty, &office_list) != 0) return 1;
    if (write_csv_summary(office_csv_path, &courses, &office_list) != 0) return 1;
    if (write_csv_courses(courses_csv_path, &courses) != 0) return 1;

    printf("Wrote report to '%s'.\n", report_path);
    printf("Wrote office-hours CSV to '%s'.\n", office_csv_path);
    printf("Wrote class-schedule CSV to '%s'.\n", courses_csv_path);

    return 0;
}
