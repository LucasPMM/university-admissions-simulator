#include "admissions_allocation.h"

#include <stdbool.h>
#include <string.h>

static const char *pending_first_choice_winner(const ApplicantNode *head, int seats,
                                                int course_index) {
    int position = 0;
    for (const ApplicantNode *candidate = head; candidate != NULL; candidate = candidate->next) {
        ++position;
        if (candidate->first_choice == course_index && position <= seats &&
            !candidate->second_choice_removed) {
            return candidate->name;
        }
    }
    return NULL;
}

static int second_choice_for_name(const ApplicantNode *head, const char *name) {
    /* Phase 3 will replace name matching with stable IDs so duplicate names are safe. */
    for (const ApplicantNode *candidate = head; candidate != NULL; candidate = candidate->next) {
        if (strcmp(candidate->name, name) == 0) {
            return candidate->second_choice;
        }
    }
    return -1;
}

static void mark_second_choice_removed(ApplicantNode *head, const char *name) {
    for (ApplicantNode *candidate = head; candidate != NULL; candidate = candidate->next) {
        if (strcmp(candidate->name, name) == 0) {
            candidate->second_choice_removed = true;
            return;
        }
    }
}

void admissions_resolve(Course **courses, size_t course_count) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t index = 0; index < course_count; ++index) {
            Course *course = courses[index];
            const char *name = pending_first_choice_winner(course->applicants, course->seats,
                                                            (int)index);
            if (name == NULL) {
                continue;
            }
            int second_choice = second_choice_for_name(course->applicants, name);
            if (second_choice >= 0) {
                courses[second_choice]->applicants =
                    applicant_remove_by_name(courses[second_choice]->applicants, name);
                mark_second_choice_removed(course->applicants, name);
                changed = true;
            }
        }
    }
}
