#include <string.h>
#include "faculty.h"

const Faculty *find_faculty_by_name(const FacultyList *list, const char *name) {
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->faculty[i].name, name) == 0) {
            return &list->faculty[i];
        }
    }
    return NULL;
}
