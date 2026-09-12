#ifndef ADMISSIONS_REPORT_H
#define ADMISSIONS_REPORT_H

#include "admissions_model.h"

#include <stdbool.h>
#include <stdio.h>

void admissions_print_course(FILE *output, const Course *course, bool has_next);

#endif
