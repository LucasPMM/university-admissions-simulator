#include "admissions_allocation.h"
#include "admissions_model.h"
#include "admissions_ranking.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>

enum { MAX_TEST_COURSES = 3, MAX_TEST_CANDIDATES = 4, SCORE_LEVELS = 3 };

typedef struct SmallCase {
    size_t course_count;
    size_t candidate_count;
    size_t seats[MAX_TEST_COURSES];
    size_t first_choice[MAX_TEST_CANDIDATES];
    size_t second_choice[MAX_TEST_CANDIDATES];
    float scores[MAX_TEST_CANDIDATES];
} SmallCase;

static size_t power(size_t base, size_t exponent) {
    size_t result = 1;
    for (size_t index = 0; index < exponent; ++index) {
        result *= base;
    }
    return result;
}

static bool oracle_precedes(const SmallCase *test_case, size_t left, size_t right,
                            size_t course_index) {
    if (test_case->scores[left] != test_case->scores[right]) {
        return test_case->scores[left] > test_case->scores[right];
    }
    bool left_first = test_case->first_choice[left] == course_index;
    bool right_first = test_case->first_choice[right] == course_index;
    if (left_first != right_first) {
        return left_first;
    }
    return left < right;
}

/* This oracle sorts candidate indices from scratch; it never inspects the linked lists. */
static size_t oracle_rank(const SmallCase *test_case, size_t course_index, unsigned removed_mask,
                          size_t ranked[MAX_TEST_CANDIDATES]) {
    size_t count = 0;
    for (size_t candidate = 0; candidate < test_case->candidate_count; ++candidate) {
        bool eligible = test_case->first_choice[candidate] == course_index ||
                        (test_case->second_choice[candidate] == course_index &&
                         (removed_mask & (1U << candidate)) == 0);
        if (!eligible) {
            continue;
        }
        size_t position = count;
        while (position > 0 &&
               oracle_precedes(test_case, candidate, ranked[position - 1], course_index)) {
            ranked[position] = ranked[position - 1];
            --position;
        }
        ranked[position] = candidate;
        ++count;
    }
    return count;
}

static unsigned first_choice_winners(const SmallCase *test_case, unsigned removed_mask) {
    unsigned winners = 0;
    for (size_t course = 0; course < test_case->course_count; ++course) {
        size_t ranked[MAX_TEST_CANDIDATES];
        size_t count = oracle_rank(test_case, course, removed_mask, ranked);
        size_t admitted_count = count < test_case->seats[course] ? count : test_case->seats[course];
        for (size_t position = 0; position < admitted_count; ++position) {
            size_t candidate = ranked[position];
            if (test_case->first_choice[candidate] == course) {
                winners |= 1U << candidate;
            }
        }
    }
    return winners;
}

/* Enumerating all fixed points avoids copying the production removal loop into the oracle. */
static unsigned least_fixed_point(const SmallCase *test_case) {
    unsigned all_candidates = (1U << test_case->candidate_count) - 1U;
    unsigned intersection = all_candidates;
    bool found = false;
    for (unsigned removed_mask = 0; removed_mask <= all_candidates; ++removed_mask) {
        if ((removed_mask | first_choice_winners(test_case, removed_mask)) == removed_mask) {
            intersection &= removed_mask;
            found = true;
        }
    }
    assert(found);
    assert((intersection | first_choice_winners(test_case, intersection)) == intersection);
    return intersection;
}

static void build_model(const SmallCase *test_case, Admissions *admissions) {
    assert(admissions_model_init(admissions, test_case->course_count, test_case->candidate_count));
    for (size_t course = 0; course < test_case->course_count; ++course) {
        admissions->courses[course].seats = test_case->seats[course];
    }
    for (size_t index = 0; index < test_case->candidate_count; ++index) {
        Candidate *candidate = &admissions->candidates[index];
        candidate->id = index;
        candidate->score = test_case->scores[index];
        candidate->first_choice = test_case->first_choice[index];
        candidate->second_choice = test_case->second_choice[index];
        int length = snprintf(candidate->name, sizeof(candidate->name), "C%zu", index);
        assert(length > 0 && (size_t)length < sizeof(candidate->name));
        ApplicationNode *first = application_node_create(candidate);
        ApplicationNode *second = application_node_create(candidate);
        assert(first != NULL && second != NULL);
        admissions_insert_ranked(&admissions->courses[candidate->first_choice], first,
                                 candidate->first_choice);
        admissions_insert_ranked(&admissions->courses[candidate->second_choice], second,
                                 candidate->second_choice);
    }
}

static void verify_model(const SmallCase *test_case, const Admissions *admissions,
                         unsigned removed_mask) {
    unsigned admitted_mask = 0;
    for (size_t index = 0; index < test_case->candidate_count; ++index) {
        bool expected_removed = (removed_mask & (1U << index)) != 0;
        assert(admissions->candidates[index].second_choice_removed == expected_removed);
    }
    for (size_t course = 0; course < test_case->course_count; ++course) {
        size_t expected[MAX_TEST_CANDIDATES];
        size_t count = oracle_rank(test_case, course, removed_mask, expected);
        const ApplicationNode *application = admissions->courses[course].applications;
        for (size_t position = 0; position < count; ++position) {
            assert(application != NULL);
            size_t candidate = application->candidate->id;
            assert(candidate == expected[position]);
            if (position < test_case->seats[course]) {
                assert((admitted_mask & (1U << candidate)) == 0);
                admitted_mask |= 1U << candidate;
            }
            application = application->next;
        }
        assert(application == NULL);
        float expected_cutoff = 0.0f;
        if (test_case->seats[course] > 0 && count >= test_case->seats[course]) {
            expected_cutoff = test_case->scores[expected[test_case->seats[course] - 1]];
        }
        assert(admissions->courses[course].cutoff == expected_cutoff);
    }
}

static void check_case(const SmallCase *test_case) {
    unsigned expected_removed = least_fixed_point(test_case);
    Admissions admissions;
    build_model(test_case, &admissions);
    admissions_resolve(&admissions);
    admissions_update_cutoffs(&admissions);
    verify_model(test_case, &admissions, expected_removed);

    /* Resolution must be idempotent after reaching the fixed point. */
    admissions_resolve(&admissions);
    admissions_update_cutoffs(&admissions);
    verify_model(test_case, &admissions, expected_removed);
    admissions_model_destroy(&admissions);
}

static size_t production_rank(const Course *course, size_t candidate_id) {
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

static void check_score_monotonicity(const SmallCase *test_case) {
    Admissions original;
    build_model(test_case, &original);
    for (size_t candidate = 0; candidate < test_case->candidate_count; ++candidate) {
        SmallCase increased = *test_case;
        increased.scores[candidate] += 1.0f;
        Admissions improved;
        build_model(&increased, &improved);
        size_t first = test_case->first_choice[candidate];
        size_t second = test_case->second_choice[candidate];
        assert(production_rank(&improved.courses[first], candidate) <=
               production_rank(&original.courses[first], candidate));
        assert(production_rank(&improved.courses[second], candidate) <=
               production_rank(&original.courses[second], candidate));
        admissions_model_destroy(&improved);
    }
    admissions_model_destroy(&original);
}

static size_t enumerate_cases(size_t course_count, size_t maximum_candidates) {
    size_t first_options[MAX_TEST_COURSES * (MAX_TEST_COURSES - 1)];
    size_t second_options[MAX_TEST_COURSES * (MAX_TEST_COURSES - 1)];
    size_t preference_count = 0;
    for (size_t first = 0; first < course_count; ++first) {
        for (size_t second = 0; second < course_count; ++second) {
            if (first != second) {
                first_options[preference_count] = first;
                second_options[preference_count] = second;
                ++preference_count;
            }
        }
    }

    size_t checked = 0;
    size_t candidate_variants = preference_count * SCORE_LEVELS;
    for (size_t candidate_count = 0; candidate_count <= maximum_candidates; ++candidate_count) {
        size_t candidate_combinations = power(candidate_variants, candidate_count);
        size_t seat_combinations = power(3, course_count);
        for (size_t candidate_code = 0; candidate_code < candidate_combinations; ++candidate_code) {
            SmallCase test_case = {.course_count = course_count,
                                   .candidate_count = candidate_count};
            size_t remaining_candidates = candidate_code;
            for (size_t index = 0; index < candidate_count; ++index) {
                size_t variant = remaining_candidates % candidate_variants;
                remaining_candidates /= candidate_variants;
                size_t preference = variant / SCORE_LEVELS;
                test_case.first_choice[index] = first_options[preference];
                test_case.second_choice[index] = second_options[preference];
                test_case.scores[index] = (float)(variant % SCORE_LEVELS);
            }
            check_score_monotonicity(&test_case);
            for (size_t seat_code = 0; seat_code < seat_combinations; ++seat_code) {
                size_t remaining_seats = seat_code;
                for (size_t course = 0; course < course_count; ++course) {
                    test_case.seats[course] = remaining_seats % 3;
                    remaining_seats /= 3;
                }
                check_case(&test_case);
                ++checked;
            }
        }
    }
    return checked;
}

int main(void) {
    size_t two_course_cases = enumerate_cases(2, 4);
    size_t three_course_cases = enumerate_cases(3, 3);
    printf("Exhaustive tests passed: %zu two-course and %zu three-course cases.\n",
           two_course_cases, three_course_cases);
    return 0;
}
