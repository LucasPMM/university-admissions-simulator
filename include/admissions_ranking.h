#ifndef ADMISSIONS_RANKING_H
#define ADMISSIONS_RANKING_H

#include "admissions_model.h"

void admissions_insert_ranked(Course *course, ApplicationNode *application, size_t course_index);
void admissions_update_cutoffs(Admissions *admissions);

#endif
