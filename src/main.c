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

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "course.h"
#include "faculty.h"
#include "parser.h"
#include "scheduler.h"


/*
 * Create the output directory.
 *
 * Windows uses _mkdir(path).
 * Linux/macOS uses mkdir(path, permissions).
 */
static void ensure_dir(const char *path) {
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}


int main(int argc, char *argv[]) {

    /*
     * The program requires:
     *
     * 1. class schedule file
     * 2. faculty information file
     *
     * An optional third argument can specify the output directory.
     */
    if (argc < 3 || argc > 4) {
        fprintf(
            stderr,
            "Usage: %s <class_schedule_file> <faculty_info_file> [output_dir]\n",
            argv[0]
        );

        return 1;
    }


    const char *class_file = argv[1];
    const char *faculty_file = argv[2];

    const char *out_dir =
        (argc == 4)
        ? argv[3]
        : "output";


    /*
     * Store all courses, faculty members,
     * and generated office-hour schedules.
     */
    static CourseList courses;
    static FacultyList faculty;
    static OfficeScheduleList office_list;


    /*
     * Read the two required input files.
     */
    if (load_course_list(class_file, &courses) != 0) {
        return 1;
    }

    if (load_faculty_list(faculty_file, &faculty) != 0) {
        return 1;
    }


    printf(
        "Loaded %d course(s) from '%s'.\n",
        courses.count,
        class_file
    );

    printf(
        "Loaded %d faculty member(s) from '%s'.\n",
        faculty.count,
        faculty_file
    );


    /*
     * Check whether every instructor in the course schedule
     * also exists in the faculty-information file.
     *
     * Office hours cannot be generated for an instructor
     * if the faculty file does not contain their required
     * weekly office-hour information.
     */
    for (int i = 0; i < courses.count; i++) {

        if (
            find_faculty_by_name(
                &faculty,
                courses.courses[i].instructor
            ) == NULL
        ) {

            fprintf(
                stderr,
                "Warning: '%s' teaches %s but has no entry in the faculty file\n",
                courses.courses[i].instructor,
                courses.courses[i].code
            );
        }
    }


    /*
     * Generate conflict-free office-hour schedules.
     */
    generate_office_schedules(
        &courses,
        &faculty,
        &office_list
    );


    /*
     * Check whether each faculty member received
     * the required amount of office-hour time.
     */
    for (int i = 0; i < office_list.count; i++) {

        const OfficeSchedule *s =
            &office_list.schedules[i];


        if (
            s->minutes_assigned
            <
            s->minutes_required
        ) {

            fprintf(
                stderr,
                "Warning: only scheduled %d of %d required office-hour minutes for %s\n",
                s->minutes_assigned,
                s->minutes_required,
                s->instructor
            );
        }
    }


    /*
     * Create the output directory if needed.
     */
    ensure_dir(out_dir);


    /*
     * Build the output file paths.
     */
    char report_path[512];
    char office_csv_path[512];
    char courses_csv_path[512];


    snprintf(
        report_path,
        sizeof(report_path),
        "%s/schedule_report.txt",
        out_dir
    );


    snprintf(
        office_csv_path,
        sizeof(office_csv_path),
        "%s/office_hours.csv",
        out_dir
    );


    snprintf(
        courses_csv_path,
        sizeof(courses_csv_path),
        "%s/class_schedule.csv",
        out_dir
    );


    /*
     * Write the text report.
     */
    if (
        write_report(
            report_path,
            &courses,
            &faculty,
            &office_list
        ) != 0
    ) {

        return 1;
    }


    /*
     * Write the office-hours CSV file.
     */
    if (
        write_csv_summary(
            office_csv_path,
            &courses,
            &office_list
        ) != 0
    ) {

        return 1;
    }


    /*
     * Write the class-schedule CSV file.
     */
    if (
        write_csv_courses(
            courses_csv_path,
            &courses
        ) != 0
    ) {

        return 1;
    }


    /*
     * Show the user where the files were created.
     */
    printf(
        "Wrote report to '%s'.\n",
        report_path
    );

    printf(
        "Wrote office-hours CSV to '%s'.\n",
        office_csv_path
    );

    printf(
        "Wrote class-schedule CSV to '%s'.\n",
        courses_csv_path
    );


    return 0;
}