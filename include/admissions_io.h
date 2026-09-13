#ifndef ADMISSIONS_IO_H
#define ADMISSIONS_IO_H

#include "admissions_model.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef enum AdmissionsInputStatus {
    ADMISSIONS_INPUT_OK,
    ADMISSIONS_INPUT_COUNTS,
    ADMISSIONS_INPUT_COURSE_NAME,
    ADMISSIONS_INPUT_SEATS,
    ADMISSIONS_INPUT_CANDIDATE_NAME,
    ADMISSIONS_INPUT_CANDIDATE_DETAILS,
    ADMISSIONS_INPUT_TRAILING,
    ADMISSIONS_INPUT_IO,
    ADMISSIONS_INPUT_MEMORY
} AdmissionsInputStatus;

typedef struct AdmissionsInputError {
    AdmissionsInputStatus status;
    size_t line;
} AdmissionsInputError;

bool admissions_parse(FILE *input, Admissions *admissions, AdmissionsInputError *error);
const char *admissions_input_status_message(AdmissionsInputStatus status);

#endif
