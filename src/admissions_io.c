#include "admissions_io.h"

#include "admissions_ranking.h"

#include <math.h>
#include <stdlib.h>

static bool read_name(FILE *input, char name[ADMISSIONS_NAME_CAPACITY]) {
    int character;
    while ((character = fgetc(input)) != '\n' && character != EOF) {
    }
    return fgets(name, ADMISSIONS_NAME_CAPACITY, input) != NULL;
}

bool admissions_read_course(FILE *input, Course **course) {
    char name[ADMISSIONS_NAME_CAPACITY];
    int seats;
    if (!read_name(input, name) || fscanf(input, "%d", &seats) != 1 || seats < 0) {
        return false;
    }
    *course = course_create(name, seats);
    return *course != NULL;
}

bool admissions_read_candidate(FILE *input, Course **courses, size_t course_count) {
    char name[ADMISSIONS_NAME_CAPACITY];
    float score;
    int first_choice;
    int second_choice;
    if (!read_name(input, name) ||
        fscanf(input, "%f%d%d", &score, &first_choice, &second_choice) != 3 ||
        !isfinite(score) || score < 0.0F || first_choice < 0 || second_choice < 0 ||
        (size_t)first_choice >= course_count || (size_t)second_choice >= course_count ||
        first_choice == second_choice) {
        return false;
    }

    ApplicantNode *first = applicant_create(name, score, first_choice, second_choice);
    ApplicantNode *second = applicant_create(name, score, first_choice, second_choice);
    if (first == NULL || second == NULL) {
        free(first);
        free(second);
        return false;
    }
    applicant_insert_ranked(&courses[first_choice]->applicants, first, first_choice);
    applicant_insert_ranked(&courses[second_choice]->applicants, second, second_choice);
    return true;
}
