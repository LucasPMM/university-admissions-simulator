#include "admissions_ranking.h"

static bool precedes(const Candidate *left, const Candidate *right, size_t course_index) {
    if (left->score != right->score) {
        return left->score > right->score;
    }
    bool left_is_first_choice = left->first_choice == course_index;
    bool right_is_first_choice = right->first_choice == course_index;
    if (left_is_first_choice != right_is_first_choice) {
        return left_is_first_choice;
    }
    return left->id < right->id;
}

void admissions_insert_ranked(Course *course, ApplicationNode *application, size_t course_index) {
    ApplicationNode **position = &course->applications;
    while (*position != NULL &&
           precedes((*position)->candidate, application->candidate, course_index)) {
        position = &(*position)->next;
    }
    application->next = *position;
    *position = application;
}

void admissions_update_cutoffs(Admissions *admissions) {
    for (size_t index = 0; index < admissions->course_count; ++index) {
        Course *course = &admissions->courses[index];
        course->cutoff = 0.0f;
        if (course->seats == 0) {
            continue;
        }
        size_t position = 0;
        for (const ApplicationNode *application = course->applications; application != NULL;
             application = application->next) {
            ++position;
            if (position == course->seats) {
                course->cutoff = application->candidate->score;
                break;
            }
        }
    }
}
