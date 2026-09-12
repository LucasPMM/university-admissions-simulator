#ifndef ADMISSIONS_RANKING_H
#define ADMISSIONS_RANKING_H

#include "admissions_model.h"

void applicant_insert_ranked(ApplicantNode **head, ApplicantNode *applicant,
                             int course_index);
void course_update_cutoff(Course *course);

#endif
