#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

#define LINE_LEN 512
#define MAX_FIELDS 8

/* Strips leading/trailing whitespace and trailing newline in place. */
static char *trim(char *s) {
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

/* Splits `line` on '|' in place, filling `fields` with pointers into `line`.
   Returns the number of fields found (up to max_fields). */
static int split_fields(char *line, char *fields[], int max_fields) {
    int n = 0;
    char *tok = strtok(line, "|");
    while (tok != NULL && n < max_fields) {
        fields[n++] = trim(tok);
        tok = strtok(NULL, "|");
    }
    return n;
}

static int is_blank_or_comment(const char *line) {
    while (isspace((unsigned char)*line)) line++;
    return (*line == '\0' || *line == '#');
}

int load_course_list(const char *filename, CourseList *list) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: could not open class schedule file '%s'\n", filename);
        return -1;
    }

    list->count = 0;
    char line[LINE_LEN];
    int lineno = 0;

    while (fgets(line, sizeof(line), fp)) {
        lineno++;
        if (is_blank_or_comment(line)) continue;

        char buf[LINE_LEN];
        strncpy(buf, line, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        char *fields[MAX_FIELDS];
        int n = split_fields(buf, fields, MAX_FIELDS);
        if (n != 7) {
            fprintf(stderr, "Warning: %s:%d: expected 7 fields, got %d (skipped)\n",
                    filename, lineno, n);
            continue;
        }
        if (list->count >= MAX_COURSES) {
            fprintf(stderr, "Warning: %s: too many courses, ignoring line %d\n", filename, lineno);
            continue;
        }

        Course *c = &list->courses[list->count];
        strncpy(c->code, fields[0], CODE_LEN - 1); c->code[CODE_LEN - 1] = '\0';
        strncpy(c->title, fields[1], TITLE_LEN - 1); c->title[TITLE_LEN - 1] = '\0';
        strncpy(c->instructor, fields[2], NAME_LEN - 1); c->instructor[NAME_LEN - 1] = '\0';
        c->days = parse_days(fields[3]);
        c->start_min = time_to_minutes(fields[4]);
        c->end_min = time_to_minutes(fields[5]);
        strncpy(c->room, fields[6], ROOM_LEN - 1); c->room[ROOM_LEN - 1] = '\0';

        if (c->days == 0 || c->start_min < 0 || c->end_min < 0 || c->end_min <= c->start_min) {
            fprintf(stderr, "Warning: %s:%d: invalid days/times for course '%s' (skipped)\n",
                    filename, lineno, c->code);
            continue;
        }

        list->count++;
    }

    fclose(fp);
    return 0;
}

int load_faculty_list(const char *filename, FacultyList *list) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: could not open faculty info file '%s'\n", filename);
        return -1;
    }

    list->count = 0;
    char line[LINE_LEN];
    int lineno = 0;

    while (fgets(line, sizeof(line), fp)) {
        lineno++;
        if (is_blank_or_comment(line)) continue;

        char buf[LINE_LEN];
        strncpy(buf, line, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        char *fields[MAX_FIELDS];
        int n = split_fields(buf, fields, MAX_FIELDS);
        if (n != 5) {
            fprintf(stderr, "Warning: %s:%d: expected 5 fields, got %d (skipped)\n",
                    filename, lineno, n);
            continue;
        }
        if (list->count >= MAX_FACULTY) {
            fprintf(stderr, "Warning: %s: too many faculty, ignoring line %d\n", filename, lineno);
            continue;
        }

        Faculty *f = &list->faculty[list->count];
        strncpy(f->name, fields[0], NAME_LEN - 1); f->name[NAME_LEN - 1] = '\0';
        strncpy(f->department, fields[1], DEPT_LEN - 1); f->department[DEPT_LEN - 1] = '\0';
        f->weekly_office_hours = atoi(fields[2]);
        strncpy(f->office_room, fields[3], ROOM_LEN - 1); f->office_room[ROOM_LEN - 1] = '\0';
        f->preferred_days = parse_days(fields[4]);

        if (f->weekly_office_hours <= 0) {
            fprintf(stderr, "Warning: %s:%d: invalid weekly office hours for '%s' (skipped)\n",
                    filename, lineno, f->name);
            continue;
        }

        list->count++;
    }

    fclose(fp);
    return 0;
}
