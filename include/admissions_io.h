#ifndef ADMISSIONS_IO_H
#define ADMISSIONS_IO_H

#include "admissions_model.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool admissions_read_course(FILE *input, Course **course);
bool admissions_read_candidate(FILE *input, Course **courses, size_t course_count);

#endif
