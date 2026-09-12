#include "admissions_ranking.h"

void applicant_insert_ranked(ApplicantNode **head, ApplicantNode *applicant,
                             int course_index) {
    ApplicantNode **current = head;
    while (*current != NULL && (*current)->score >= applicant->score) {
        /* On a score tie, first-choice applicants precede second-choice applicants. */
        if ((*current)->score == applicant->score &&
            (*current)->second_choice == course_index &&
            applicant->first_choice == course_index) {
            break;
        }
        current = &(*current)->next;
    }
    applicant->next = *current;
    *current = applicant;
}

void course_update_cutoff(Course *course) {
    course->cutoff = 0.0F;
    int position = 0;
    for (const ApplicantNode *applicant = course->applicants; applicant != NULL;
         applicant = applicant->next) {
        ++position;
        if (position == course->seats) {
            course->cutoff = applicant->score;
            break;
        }
    }
}
