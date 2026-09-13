#include "admissions_io.h"
#include "admissions_model.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_TRACKED_ALLOCATIONS = 64 };

static void *tracked[MAX_TRACKED_ALLOCATIONS];
static size_t tracked_count;
static size_t allocation_count;
static size_t fail_at;

void *__real_malloc(size_t size);
void *__real_calloc(size_t count, size_t size);
void *__real_realloc(void *pointer, size_t size);
void __real_free(void *pointer);

static bool should_fail(void) {
    ++allocation_count;
    return fail_at != 0 && allocation_count == fail_at;
}

static void track(void *pointer) {
    if (pointer != NULL) {
        assert(tracked_count < MAX_TRACKED_ALLOCATIONS);
        tracked[tracked_count++] = pointer;
    }
}

static void untrack(void *pointer) {
    if (pointer == NULL) {
        return;
    }
    for (size_t index = 0; index < tracked_count; ++index) {
        if (tracked[index] == pointer) {
            tracked[index] = tracked[--tracked_count];
            return;
        }
    }
    assert(false);
}

void *__wrap_malloc(size_t size) {
    if (should_fail()) {
        return NULL;
    }
    void *pointer = __real_malloc(size);
    track(pointer);
    return pointer;
}

void *__wrap_calloc(size_t count, size_t size) {
    if (should_fail()) {
        return NULL;
    }
    void *pointer = __real_calloc(count, size);
    track(pointer);
    return pointer;
}

void *__wrap_realloc(void *pointer, size_t size) {
    if (should_fail()) {
        return NULL;
    }
    void *resized = __real_realloc(pointer, size);
    if (resized != NULL) {
        untrack(pointer);
        track(resized);
    }
    return resized;
}

void __wrap_free(void *pointer) {
    untrack(pointer);
    __real_free(pointer);
}

static FILE *input_from_text(const char *text) {
    FILE *input = tmpfile();
    assert(input != NULL);
    assert(fputs(text, input) >= 0);
    rewind(input);
    return input;
}

static void test_every_allocation_failure(void) {
    char input_text[512];
    memset(input_text, ' ', 200);
    const char *records = "2 2\nA\n1\nB\n1\nAlex\n90 0 1\nBlair\n80 1 0\n";
    assert(200 + strlen(records) + 1 <= sizeof(input_text));
    memcpy(input_text + 200, records, strlen(records) + 1);

    FILE *input = input_from_text(input_text);
    Admissions admissions;
    AdmissionsInputError error;
    assert(admissions_parse(input, &admissions, &error));
    size_t total_allocations = allocation_count;
    assert(total_allocations >= 8);
    admissions_model_destroy(&admissions);
    assert(tracked_count == 0);
    fclose(input);

    for (size_t failure = 1; failure <= total_allocations; ++failure) {
        input = input_from_text(input_text);
        allocation_count = 0;
        fail_at = failure;
        assert(!admissions_parse(input, &admissions, &error));
        fail_at = 0;
        assert(error.status == ADMISSIONS_INPUT_MEMORY);
        assert(admissions.courses == NULL);
        assert(admissions.candidates == NULL);
        assert(tracked_count == 0);
        fclose(input);
    }
    printf("Allocation cleanup passed at all %zu failure points.\n", total_allocations);
}

static void test_empty_line_allocation_failure(void) {
    FILE *input = input_from_text("\n");
    Admissions admissions;
    AdmissionsInputError error;
    allocation_count = 0;
    fail_at = 1;
    assert(!admissions_parse(input, &admissions, &error));
    fail_at = 0;
    assert(error.status == ADMISSIONS_INPUT_MEMORY);
    assert(tracked_count == 0);
    fclose(input);
}

int main(void) {
    test_every_allocation_failure();
    test_empty_line_allocation_failure();
    return 0;
}
