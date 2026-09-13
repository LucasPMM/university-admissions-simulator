#include "admissions_model.h"

#include <stdlib.h>

bool admissions_model_init(Admissions *admissions, size_t course_count, size_t candidate_count) {
    admissions->courses = NULL;
    admissions->course_count = course_count;
    admissions->candidates = NULL;
    admissions->candidate_count = candidate_count;

    if (course_count != 0) {
        admissions->courses = calloc(course_count, sizeof(*admissions->courses));
        if (admissions->courses == NULL) {
            admissions_model_destroy(admissions);
            return false;
        }
    }
    if (candidate_count != 0) {
        admissions->candidates = calloc(candidate_count, sizeof(*admissions->candidates));
        if (admissions->candidates == NULL) {
            admissions_model_destroy(admissions);
            return false;
        }
    }
    return true;
}

ApplicationNode *application_node_create(Candidate *candidate) {
    ApplicationNode *application = calloc(1, sizeof(*application));
    if (application != NULL) {
        application->candidate = candidate;
    }
    return application;
}

void admissions_model_destroy(Admissions *admissions) {
    if (admissions->courses != NULL) {
        for (size_t index = 0; index < admissions->course_count; ++index) {
            ApplicationNode *application = admissions->courses[index].applications;
            while (application != NULL) {
                ApplicationNode *next = application->next;
                free(application);
                application = next;
            }
        }
    }
    free(admissions->courses);
    free(admissions->candidates);
    admissions->courses = NULL;
    admissions->course_count = 0;
    admissions->candidates = NULL;
    admissions->candidate_count = 0;
}
