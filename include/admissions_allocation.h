#ifndef ADMISSIONS_ALLOCATION_H
#define ADMISSIONS_ALLOCATION_H

#include "admissions_model.h"

#include <stddef.h>

void admissions_resolve(Course **courses, size_t course_count);

#endif
