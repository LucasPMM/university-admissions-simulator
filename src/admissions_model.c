#include "admissions_model.h"

#include <stdlib.h>
#include <string.h>

static void copy_name(char destination[ADMISSIONS_NAME_CAPACITY], const char *source) {
    size_t length = strcspn(source, "\r\n");
    if (length >= ADMISSIONS_NAME_CAPACITY) {
        length = ADMISSIONS_NAME_CAPACITY - 1;
    }
    memcpy(destination, source, length);
    destination[length] = '\0';
}

Course *course_create(const char *name, int seats) {
    Course *course = calloc(1, sizeof(*course));
    if (course == NULL) {
        return NULL;
    }
    copy_name(course->name, name);
    course->seats = seats;
    return course;
}

ApplicantNode *applicant_create(const char *name, float score, int first_choice,
                                int second_choice) {
    ApplicantNode *applicant = calloc(1, sizeof(*applicant));
    if (applicant == NULL) {
        return NULL;
    }
    copy_name(applicant->name, name);
    applicant->score = score;
    applicant->first_choice = first_choice;
    applicant->second_choice = second_choice;
    return applicant;
}

static void applicant_list_destroy(ApplicantNode *head) {
    while (head != NULL) {
        ApplicantNode *next = head->next;
        free(head);
        head = next;
    }
}

void courses_destroy(Course **courses, size_t course_count) {
    for (size_t index = 0; index < course_count; ++index) {
        if (courses[index] != NULL) {
            applicant_list_destroy(courses[index]->applicants);
            free(courses[index]);
        }
    }
    free(courses);
}

ApplicantNode *applicant_remove_by_name(ApplicantNode *head, const char *name) {
    ApplicantNode **current = &head;
    while (*current != NULL && strcmp((*current)->name, name) != 0) {
        current = &(*current)->next;
    }
    if (*current != NULL) {
        ApplicantNode *removed = *current;
        *current = removed->next;
        free(removed);
    }
    return head;
}
