#ifndef ADMISSIONS_MODEL_H
#define ADMISSIONS_MODEL_H

#include <stdbool.h>
#include <stddef.h>

enum { ADMISSIONS_NAME_CAPACITY = 100 };

typedef struct ApplicantNode {
    char name[ADMISSIONS_NAME_CAPACITY];
    int first_choice;
    int second_choice;
    bool second_choice_removed;
    float score;
    struct ApplicantNode *next;
} ApplicantNode;

typedef struct Course {
    char name[ADMISSIONS_NAME_CAPACITY];
    int seats;
    float cutoff;
    ApplicantNode *applicants;
} Course;

/* Callers own new objects. A course owns applicant nodes once they are inserted into its list. */
Course *course_create(const char *name, int seats);
ApplicantNode *applicant_create(const char *name, float score, int first_choice,
                                int second_choice);
void courses_destroy(Course **courses, size_t course_count);
ApplicantNode *applicant_remove_by_name(ApplicantNode *head, const char *name);

#endif
