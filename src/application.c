#include "application.h"

#include "admissions_allocation.h"
#include "admissions_io.h"
#include "admissions_model.h"
#include "admissions_ranking.h"
#include "admissions_report.h"

#include <stdlib.h>

enum { MAX_COURSES = 1000, MAX_CANDIDATES = 100000 };

int admissions_run(FILE *input, FILE *output, FILE *error) {
    int course_count;
    int candidate_count;
    if (fscanf(input, "%d%d", &course_count, &candidate_count) != 2 || course_count < 0 ||
        course_count > MAX_COURSES || candidate_count < 0 || candidate_count > MAX_CANDIDATES ||
        (course_count == 0 && candidate_count != 0)) {
        fputs("Invalid course or candidate count.\n", error);
        return EXIT_FAILURE;
    }
    if (course_count == 0) {
        return EXIT_SUCCESS;
    }

    Course **courses = calloc((size_t)course_count, sizeof(*courses));
    if (courses == NULL) {
        fputs("Unable to allocate courses.\n", error);
        return EXIT_FAILURE;
    }

    int result = EXIT_FAILURE;
    for (int index = 0; index < course_count; ++index) {
        if (!admissions_read_course(input, &courses[index])) {
            fputs("Invalid course input or insufficient memory.\n", error);
            goto cleanup;
        }
    }
    for (int index = 0; index < candidate_count; ++index) {
        if (!admissions_read_candidate(input, courses, (size_t)course_count)) {
            fputs("Invalid candidate input or insufficient memory.\n", error);
            goto cleanup;
        }
    }

    admissions_resolve(courses, (size_t)course_count);
    for (int index = 0; index < course_count; ++index) {
        course_update_cutoff(courses[index]);
        admissions_print_course(output, courses[index], index + 1 < course_count);
    }
    result = EXIT_SUCCESS;

cleanup:
    courses_destroy(courses, (size_t)course_count);
    return result;
}
