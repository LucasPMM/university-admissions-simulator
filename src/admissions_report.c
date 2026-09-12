#include "admissions_report.h"

void admissions_print_course(FILE *output, const Course *course, bool has_next) {
    fprintf(output, "%s %.2f\nAdmitted\n", course->name, (double)course->cutoff);
    bool waiting_printed = false;
    int position = 0;
    for (const ApplicantNode *candidate = course->applicants; candidate != NULL;
         candidate = candidate->next) {
        if (!waiting_printed && position == course->seats) {
            fputs("Waiting list\n", output);
            waiting_printed = true;
        }
        fprintf(output, "%s %.2f\n", candidate->name, (double)candidate->score);
        ++position;
    }
    if (!waiting_printed) {
        fputs("Waiting list\n", output);
    }
    if (has_next) {
        fputc('\n', output);
    }
}
