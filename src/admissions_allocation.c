#include "admissions_allocation.h"

#include <stdlib.h>

static bool remove_application(Course *course, size_t candidate_id) {
    ApplicationNode **position = &course->applications;
    while (*position != NULL && (*position)->candidate->id != candidate_id) {
        position = &(*position)->next;
    }
    if (*position == NULL) {
        return false;
    }
    ApplicationNode *removed = *position;
    *position = removed->next;
    free(removed);
    return true;
}

void admissions_resolve(Admissions *admissions) {
    bool changed;
    do {
        changed = false;
        for (size_t course_index = 0; course_index < admissions->course_count; ++course_index) {
            Course *course = &admissions->courses[course_index];
            size_t position = 0;
            for (ApplicationNode *application = course->applications;
                 application != NULL && position < course->seats;
                 application = application->next, ++position) {
                Candidate *candidate = application->candidate;
                if (candidate->first_choice != course_index || candidate->second_choice_removed) {
                    continue;
                }

                /* Removals only promote candidates, so the process reaches a fixed point. */
                Course *second_course = &admissions->courses[candidate->second_choice];
                remove_application(second_course, candidate->id);
                candidate->second_choice_removed = true;
                changed = true;
            }
        }
    } while (changed);
}
