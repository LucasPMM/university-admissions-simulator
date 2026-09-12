#include "admissions_report.h"

static void print_course(FILE *output, const Course *course, bool has_next) {
    fprintf(output, "%s %.2f\nAdmitted\n", course->name, course->cutoff);
    bool waiting_printed = false;
    size_t position = 0;
    for (const ApplicationNode *application = course->applications; application != NULL;
         application = application->next) {
        if (!waiting_printed && position == course->seats) {
            fputs("Waiting list\n", output);
            waiting_printed = true;
        }
        const Candidate *candidate = application->candidate;
        fprintf(output, "%s %.2f\n", candidate->name, candidate->score);
        ++position;
    }
    if (!waiting_printed) {
        fputs("Waiting list\n", output);
    }
    if (has_next) {
        fputc('\n', output);
    }
}

bool admissions_print_report(FILE *output, const Admissions *admissions) {
    for (size_t index = 0; index < admissions->course_count; ++index) {
        print_course(output, &admissions->courses[index], index + 1 < admissions->course_count);
    }
    return fflush(output) == 0 && ferror(output) == 0;
}
