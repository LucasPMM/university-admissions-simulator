#ifndef ADMISSIONS_MODEL_H
#define ADMISSIONS_MODEL_H

#include <stdbool.h>
#include <stddef.h>

enum {
    ADMISSIONS_NAME_CAPACITY = 100,
    ADMISSIONS_MAX_COURSES = 1000,
    ADMISSIONS_MAX_CANDIDATES = 100000
};

typedef struct Candidate {
    size_t id;
    char name[ADMISSIONS_NAME_CAPACITY];
    double score;
    size_t first_choice;
    size_t second_choice;
    bool second_choice_removed;
} Candidate;

/* Each application references one authoritative candidate owned by Admissions. */
typedef struct ApplicationNode {
    Candidate *candidate;
    struct ApplicationNode *next;
} ApplicationNode;

typedef struct Course {
    char name[ADMISSIONS_NAME_CAPACITY];
    size_t seats;
    double cutoff;
    ApplicationNode *applications;
} Course;

typedef struct Admissions {
    Course *courses;
    size_t course_count;
    Candidate *candidates;
    size_t candidate_count;
} Admissions;

bool admissions_model_init(Admissions *admissions, size_t course_count, size_t candidate_count);
void admissions_model_destroy(Admissions *admissions);
ApplicationNode *application_node_create(Candidate *candidate);

#endif
