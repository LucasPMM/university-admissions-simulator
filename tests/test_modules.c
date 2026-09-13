#include "admissions_allocation.h"
#include "admissions_io.h"
#include "admissions_model.h"
#include "admissions_ranking.h"
#include "admissions_report.h"

#undef NDEBUG
#include <assert.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static FILE *input_from_text(const char *text) {
    FILE *input = tmpfile();
    assert(input != NULL);
    assert(fputs(text, input) >= 0);
    rewind(input);
    return input;
}

static void check_file_equal(FILE *actual, FILE *expected) {
    rewind(actual);
    rewind(expected);
    int actual_character;
    int expected_character;
    do {
        actual_character = fgetc(actual);
        expected_character = fgetc(expected);
        assert(actual_character == expected_character);
    } while (actual_character != EOF);
    assert(!ferror(actual));
    assert(!ferror(expected));
}

static void test_model_lifecycle(void) {
    Admissions admissions;
    assert(admissions_model_init(&admissions, 0, 0));
    assert(admissions.courses == NULL);
    assert(admissions.candidates == NULL);
    admissions_model_destroy(&admissions);

    assert(admissions_model_init(&admissions, 2, 1));
    Candidate *candidate = &admissions.candidates[0];
    candidate->id = 0;
    candidate->score = 10.0;
    candidate->first_choice = 0;
    candidate->second_choice = 1;
    ApplicationNode *first = application_node_create(candidate);
    ApplicationNode *second = application_node_create(candidate);
    assert(first != NULL && second != NULL);
    assert(first->candidate == second->candidate);
    admissions_insert_ranked(&admissions.courses[0], first, 0);
    admissions_insert_ranked(&admissions.courses[1], second, 1);
    admissions_model_destroy(&admissions);
    assert(admissions.courses == NULL);
    assert(admissions.candidates == NULL);
}

static size_t rank_of(const Course *course, size_t candidate_id) {
    size_t position = 0;
    for (const ApplicationNode *application = course->applications; application != NULL;
         application = application->next, ++position) {
        if (application->candidate->id == candidate_id) {
            return position;
        }
    }
    assert(false);
    return 0;
}

static void make_ranking(Admissions *admissions, double changed_score) {
    assert(admissions_model_init(admissions, 2, 4));
    const double scores[] = {changed_score, 90.0, 90.0, 95.0};
    const size_t first_choices[] = {1, 0, 0, 1};
    for (size_t index = 0; index < 4; ++index) {
        Candidate *candidate = &admissions->candidates[index];
        candidate->id = index;
        candidate->score = scores[index];
        candidate->first_choice = first_choices[index];
        candidate->second_choice = 1 - first_choices[index];
        ApplicationNode *application = application_node_create(candidate);
        assert(application != NULL);
        admissions_insert_ranked(&admissions->courses[0], application, 0);
    }
}

static void test_ranking_and_cutoffs(void) {
    Admissions admissions;
    make_ranking(&admissions, 90.0);
    const size_t expected_order[] = {3, 1, 2, 0};
    for (size_t position = 0; position < 4; ++position) {
        assert(rank_of(&admissions.courses[0], expected_order[position]) == position);
    }

    admissions.courses[0].seats = 2;
    admissions_update_cutoffs(&admissions);
    assert(admissions.courses[0].cutoff == 90.0);
    admissions.courses[0].seats = 5;
    admissions_update_cutoffs(&admissions);
    assert(admissions.courses[0].cutoff == 0.0);
    admissions.courses[0].seats = 0;
    admissions_update_cutoffs(&admissions);
    assert(admissions.courses[0].cutoff == 0.0);
    size_t original_rank = rank_of(&admissions.courses[0], 0);
    admissions_model_destroy(&admissions);

    make_ranking(&admissions, 96.0);
    assert(rank_of(&admissions.courses[0], 0) < original_rank);
    admissions_model_destroy(&admissions);
}

static void assert_course_ids(const Course *course, const size_t *expected, size_t count) {
    const ApplicationNode *application = course->applications;
    for (size_t position = 0; position < count; ++position) {
        assert(application != NULL);
        assert(application->candidate->id == expected[position]);
        application = application->next;
    }
    assert(application == NULL);
}

static void test_cascading_allocation(void) {
    FILE *input = fopen("tests/fixtures/cascading.in", "r");
    assert(input != NULL);
    Admissions admissions;
    AdmissionsInputError error;
    assert(admissions_parse(input, &admissions, &error));
    fclose(input);

    admissions_resolve(&admissions);
    admissions_update_cutoffs(&admissions);
    const size_t astronomy[] = {0, 3};
    const size_t biology[] = {1, 3};
    const size_t chemistry[] = {2};
    assert_course_ids(&admissions.courses[0], astronomy, 2);
    assert_course_ids(&admissions.courses[1], biology, 2);
    assert_course_ids(&admissions.courses[2], chemistry, 1);
    assert(admissions.courses[0].cutoff == 100.0);
    assert(admissions.courses[1].cutoff == 95.0);
    assert(admissions.courses[2].cutoff == 90.0);
    assert(admissions.candidates[0].second_choice_removed);
    assert(admissions.candidates[1].second_choice_removed);
    assert(admissions.candidates[2].second_choice_removed);
    assert(!admissions.candidates[3].second_choice_removed);

    admissions_resolve(&admissions);
    assert_course_ids(&admissions.courses[0], astronomy, 2);
    assert_course_ids(&admissions.courses[1], biology, 2);
    assert_course_ids(&admissions.courses[2], chemistry, 1);
    admissions_model_destroy(&admissions);
}

static void test_parser_and_report(void) {
    FILE *input = input_from_text("2 2\nA\n1\nB\n1\nAlex\n90 0 1\nAlex\n80 1 0\n");
    Admissions admissions;
    AdmissionsInputError error;
    assert(admissions_parse(input, &admissions, &error));
    assert(admissions.candidates[0].id != admissions.candidates[1].id);
    assert(strcmp(admissions.candidates[0].name, admissions.candidates[1].name) == 0);
    admissions_model_destroy(&admissions);
    fclose(input);

    input = input_from_text("2 1\nA\n1\nB\n1\nAlex\nnan 0 1\n");
    assert(!admissions_parse(input, &admissions, &error));
    assert(error.status == ADMISSIONS_INPUT_CANDIDATE_DETAILS);
    assert(error.line == 7);
    assert(admissions.courses == NULL);
    assert(admissions.candidates == NULL);
    fclose(input);

    char boundary_input[256];
    int written = snprintf(boundary_input, sizeof(boundary_input),
                           "2 1\nA\n%zu\nB\n0\nX\n%.17g 0 1\n", SIZE_MAX, DBL_MAX);
    assert(written > 0 && (size_t)written < sizeof(boundary_input));
    input = input_from_text(boundary_input);
    assert(admissions_parse(input, &admissions, &error));
    assert(admissions.courses[0].seats == SIZE_MAX);
    assert(admissions.candidates[0].score == DBL_MAX);
    admissions_model_destroy(&admissions);
    fclose(input);

    input = fopen("tests/fixtures/basic.in", "r");
    FILE *expected = fopen("tests/fixtures/basic.out", "r");
    FILE *actual = tmpfile();
    assert(input != NULL && expected != NULL && actual != NULL);
    assert(admissions_parse(input, &admissions, &error));
    admissions_resolve(&admissions);
    admissions_update_cutoffs(&admissions);
    assert(admissions_print_report(actual, &admissions));
    check_file_equal(actual, expected);
    admissions_model_destroy(&admissions);
    fclose(input);
    fclose(expected);
    fclose(actual);
}

int main(void) {
    test_model_lifecycle();
    test_ranking_and_cutoffs();
    test_cascading_allocation();
    test_parser_and_report();
    puts("Module tests passed.");
    return 0;
}
