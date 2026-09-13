#include "application.h"

#include "admissions_allocation.h"
#include "admissions_io.h"
#include "admissions_model.h"
#include "admissions_ranking.h"
#include "admissions_report.h"

#include <stdlib.h>

int admissions_run(FILE *input, FILE *output, FILE *error) {
    Admissions admissions;
    AdmissionsInputError input_error;
    if (!admissions_parse(input, &admissions, &input_error)) {
        fprintf(error, "Input error on line %zu: %s.\n", input_error.line,
                admissions_input_status_message(input_error.status));
        return EXIT_FAILURE;
    }

    admissions_resolve(&admissions);
    admissions_update_cutoffs(&admissions);
    bool report_written = admissions_print_report(output, &admissions);
    admissions_model_destroy(&admissions);
    if (!report_written) {
        fputs("Unable to write the admissions report.\n", error);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
