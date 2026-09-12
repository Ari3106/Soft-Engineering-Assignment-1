#ifndef PARSER_H
#define PARSER_H

#include "course.h"
#include "faculty.h"

/* Loads the pipe-delimited class-schedule file into `list`.
   Returns 0 on success, -1 on failure (file not found, malformed line, etc.). */
int load_course_list(const char *filename, CourseList *list);

/* Loads the pipe-delimited faculty-information file into `list`.
   Returns 0 on success, -1 on failure. */
int load_faculty_list(const char *filename, FacultyList *list);

#endif /* PARSER_H */
