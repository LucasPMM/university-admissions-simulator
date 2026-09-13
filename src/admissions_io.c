#include "admissions_io.h"

#include "admissions_ranking.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum LineStatus { LINE_OK, LINE_END, LINE_INVALID, LINE_IO, LINE_MEMORY } LineStatus;

typedef struct Parser {
    FILE *input;
    char *line;
    size_t capacity;
    size_t line_number;
    Admissions *admissions;
    AdmissionsInputError *error;
} Parser;

static bool fail(Parser *parser, AdmissionsInputStatus status, size_t line) {
    parser->error->status = status;
    parser->error->line = line;
    return false;
}

static LineStatus read_line(Parser *parser) {
    size_t length = 0;
    bool saw_character = false;
    bool saw_nul = false;
    int character;
    while ((character = fgetc(parser->input)) != EOF) {
        saw_character = true;
        if (character == '\n') {
            break;
        }
        if (character == '\0') {
            saw_nul = true;
            continue;
        }
        if (length + 1 >= parser->capacity) {
            size_t next_capacity = parser->capacity == 0 ? 128 : parser->capacity * 2;
            if (next_capacity <= parser->capacity) {
                return LINE_MEMORY;
            }
            char *next_line = realloc(parser->line, next_capacity);
            if (next_line == NULL) {
                return LINE_MEMORY;
            }
            parser->line = next_line;
            parser->capacity = next_capacity;
        }
        parser->line[length++] = (char)character;
    }
    if (ferror(parser->input)) {
        return LINE_IO;
    }
    if (!saw_character && character == EOF) {
        return LINE_END;
    }
    ++parser->line_number;
    if (parser->line == NULL) {
        parser->line = malloc(1);
        if (parser->line == NULL) {
            return LINE_MEMORY;
        }
        parser->capacity = 1;
    }
    if (length > 0 && parser->line[length - 1] == '\r') {
        --length;
    }
    parser->line[length] = '\0';
    return saw_nul ? LINE_INVALID : LINE_OK;
}

static bool require_line(Parser *parser, AdmissionsInputStatus category) {
    LineStatus status = read_line(parser);
    if (status == LINE_OK) {
        return true;
    }
    if (status == LINE_MEMORY) {
        return fail(parser, ADMISSIONS_INPUT_MEMORY, parser->line_number + 1);
    }
    if (status == LINE_IO) {
        return fail(parser, ADMISSIONS_INPUT_IO, parser->line_number + 1);
    }
    return fail(parser, category,
                status == LINE_END ? parser->line_number + 1 : parser->line_number);
}

static char *skip_spaces(char *cursor) {
    while (*cursor != '\0' && isspace((unsigned char)*cursor)) {
        ++cursor;
    }
    return cursor;
}

static bool at_line_end(char *cursor) {
    return *skip_spaces(cursor) == '\0';
}

static bool parse_unsigned(char **cursor, size_t maximum, size_t *value) {
    char *start = skip_spaces(*cursor);
    if (*start == '+') {
        ++start;
    }
    if (!isdigit((unsigned char)*start)) {
        return false;
    }
    errno = 0;
    char *end;
    unsigned long long parsed = strtoull(start, &end, 10);
    if (errno == ERANGE || parsed > maximum || (*end != '\0' && !isspace((unsigned char)*end))) {
        return false;
    }
    *value = (size_t)parsed;
    *cursor = end;
    return true;
}

static bool parse_score(char **cursor, double *score) {
    char *start = skip_spaces(*cursor);
    char *end = start;
    if (*end == '+') {
        ++end;
    }
    bool has_digit = false;
    while (isdigit((unsigned char)*end)) {
        has_digit = true;
        ++end;
    }
    if (*end == '.') {
        ++end;
        while (isdigit((unsigned char)*end)) {
            has_digit = true;
            ++end;
        }
    }
    if (!has_digit) {
        return false;
    }
    if (*end == 'e' || *end == 'E') {
        ++end;
        if (*end == '+' || *end == '-') {
            ++end;
        }
        if (!isdigit((unsigned char)*end)) {
            return false;
        }
        while (isdigit((unsigned char)*end)) {
            ++end;
        }
    }
    if (*end != '\0' && !isspace((unsigned char)*end)) {
        return false;
    }
    errno = 0;
    char *converted_end;
    double parsed = strtod(start, &converted_end);
    if (converted_end != end || !isfinite(parsed) || parsed < 0.0 ||
        (errno == ERANGE && parsed == 0.0)) {
        return false;
    }
    *score = parsed;
    *cursor = end;
    return true;
}

static bool valid_name(const char *name) {
    size_t length = strlen(name);
    if (length == 0 || length >= ADMISSIONS_NAME_CAPACITY) {
        return false;
    }
    bool has_visible_character = false;
    for (const unsigned char *character = (const unsigned char *)name; *character != '\0';
         ++character) {
        if (*character < 32 || *character == 127) {
            return false;
        }
        if (!isspace(*character)) {
            has_visible_character = true;
        }
    }
    return has_visible_character;
}

static bool parse_header(Parser *parser) {
    if (!require_line(parser, ADMISSIONS_INPUT_COUNTS)) {
        return false;
    }
    char *cursor = parser->line;
    size_t course_count;
    size_t candidate_count;
    if (!parse_unsigned(&cursor, ADMISSIONS_MAX_COURSES, &course_count) ||
        !parse_unsigned(&cursor, ADMISSIONS_MAX_CANDIDATES, &candidate_count) ||
        !at_line_end(cursor) || (course_count == 0 && candidate_count != 0)) {
        return fail(parser, ADMISSIONS_INPUT_COUNTS, parser->line_number);
    }
    if (!admissions_model_init(parser->admissions, course_count, candidate_count)) {
        return fail(parser, ADMISSIONS_INPUT_MEMORY, parser->line_number);
    }
    return true;
}

static bool parse_course(Parser *parser, size_t index) {
    if (!require_line(parser, ADMISSIONS_INPUT_COURSE_NAME)) {
        return false;
    }
    if (!valid_name(parser->line)) {
        return fail(parser, ADMISSIONS_INPUT_COURSE_NAME, parser->line_number);
    }
    Course *course = &parser->admissions->courses[index];
    memcpy(course->name, parser->line, strlen(parser->line) + 1);

    if (!require_line(parser, ADMISSIONS_INPUT_SEATS)) {
        return false;
    }
    char *cursor = parser->line;
    if (!parse_unsigned(&cursor, SIZE_MAX, &course->seats) || !at_line_end(cursor)) {
        return fail(parser, ADMISSIONS_INPUT_SEATS, parser->line_number);
    }
    return true;
}

static bool parse_candidate(Parser *parser, size_t index) {
    if (!require_line(parser, ADMISSIONS_INPUT_CANDIDATE_NAME)) {
        return false;
    }
    if (!valid_name(parser->line)) {
        return fail(parser, ADMISSIONS_INPUT_CANDIDATE_NAME, parser->line_number);
    }
    Candidate *candidate = &parser->admissions->candidates[index];
    candidate->id = index;
    memcpy(candidate->name, parser->line, strlen(parser->line) + 1);

    if (!require_line(parser, ADMISSIONS_INPUT_CANDIDATE_DETAILS)) {
        return false;
    }
    char *cursor = parser->line;
    size_t last_course = parser->admissions->course_count - 1;
    if (!parse_score(&cursor, &candidate->score) ||
        !parse_unsigned(&cursor, last_course, &candidate->first_choice) ||
        !parse_unsigned(&cursor, last_course, &candidate->second_choice) || !at_line_end(cursor) ||
        candidate->first_choice == candidate->second_choice) {
        return fail(parser, ADMISSIONS_INPUT_CANDIDATE_DETAILS, parser->line_number);
    }

    ApplicationNode *first = application_node_create(candidate);
    ApplicationNode *second = application_node_create(candidate);
    if (first == NULL || second == NULL) {
        free(first);
        free(second);
        return fail(parser, ADMISSIONS_INPUT_MEMORY, parser->line_number);
    }
    admissions_insert_ranked(&parser->admissions->courses[candidate->first_choice], first,
                             candidate->first_choice);
    admissions_insert_ranked(&parser->admissions->courses[candidate->second_choice], second,
                             candidate->second_choice);
    return true;
}

static bool check_trailing_lines(Parser *parser) {
    LineStatus status;
    while ((status = read_line(parser)) == LINE_OK) {
        if (!at_line_end(parser->line)) {
            return fail(parser, ADMISSIONS_INPUT_TRAILING, parser->line_number);
        }
    }
    if (status == LINE_END) {
        return true;
    }
    if (status == LINE_MEMORY) {
        return fail(parser, ADMISSIONS_INPUT_MEMORY, parser->line_number + 1);
    }
    if (status == LINE_IO) {
        return fail(parser, ADMISSIONS_INPUT_IO, parser->line_number + 1);
    }
    return fail(parser, ADMISSIONS_INPUT_TRAILING, parser->line_number);
}

bool admissions_parse(FILE *input, Admissions *admissions, AdmissionsInputError *error) {
    *admissions = (Admissions){0};
    *error = (AdmissionsInputError){ADMISSIONS_INPUT_OK, 0};
    Parser parser = {.input = input, .admissions = admissions, .error = error};
    bool success = parse_header(&parser);
    for (size_t index = 0; success && index < admissions->course_count; ++index) {
        success = parse_course(&parser, index);
    }
    for (size_t index = 0; success && index < admissions->candidate_count; ++index) {
        success = parse_candidate(&parser, index);
    }
    if (success) {
        success = check_trailing_lines(&parser);
    }
    free(parser.line);
    if (!success) {
        admissions_model_destroy(admissions);
    }
    return success;
}

const char *admissions_input_status_message(AdmissionsInputStatus status) {
    switch (status) {
    case ADMISSIONS_INPUT_COUNTS:
        return "invalid course or candidate count";
    case ADMISSIONS_INPUT_COURSE_NAME:
        return "invalid course name";
    case ADMISSIONS_INPUT_SEATS:
        return "invalid seat count";
    case ADMISSIONS_INPUT_CANDIDATE_NAME:
        return "invalid candidate name";
    case ADMISSIONS_INPUT_CANDIDATE_DETAILS:
        return "invalid score or course preferences";
    case ADMISSIONS_INPUT_TRAILING:
        return "unexpected trailing data";
    case ADMISSIONS_INPUT_IO:
        return "unable to read input";
    case ADMISSIONS_INPUT_MEMORY:
        return "insufficient memory";
    case ADMISSIONS_INPUT_OK:
        return "no error";
    }
    return "unknown input error";
}
