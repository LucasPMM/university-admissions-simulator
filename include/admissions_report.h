#ifndef ADMISSIONS_REPORT_H
#define ADMISSIONS_REPORT_H

#include "admissions_model.h"

#include <stdbool.h>
#include <stdio.h>

bool admissions_print_report(FILE *output, const Admissions *admissions);

#endif
