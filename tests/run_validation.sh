#!/bin/sh
set -eu

binary=$1
fixtures=$2
temporary_dir=$(mktemp -d)
trap 'rm -rf "$temporary_dir"' EXIT HUP INT TERM

check_invalid() {
    label=$1
    line=$2
    category=$3
    input_text=$4
    printf '%b' "$input_text" > "$temporary_dir/input"
    if "$binary" < "$temporary_dir/input" > "$temporary_dir/actual.out" \
        2> "$temporary_dir/actual.err"; then
        printf 'Expected failure in %s\n' "$label" >&2
        exit 1
    fi
    if [ -s "$temporary_dir/actual.out" ]; then
        printf 'Unexpected stdout in %s\n' "$label" >&2
        exit 1
    fi
    printf 'Input error on line %s: %s.\n' "$line" "$category" > "$temporary_dir/expected.err"
    diff -u "$temporary_dir/expected.err" "$temporary_dir/actual.err"
    printf 'PASS %s\n' "$label"
}

check_valid() {
    label=$1
    expected=$2
    input_text=$3
    printf '%b' "$input_text" > "$temporary_dir/input"
    "$binary" < "$temporary_dir/input" > "$temporary_dir/actual.out" \
        2> "$temporary_dir/actual.err"
    diff -u "$expected" "$temporary_dir/actual.out"
    test ! -s "$temporary_dir/actual.err"
    printf 'PASS %s\n' "$label"
}

two_courses='2 1\nA\n1\nB\n1\n'
long_name=$(awk 'BEGIN { for (i = 0; i < 100; ++i) printf "N" }')
maximum_name=$(awk 'BEGIN { for (i = 0; i < 99; ++i) printf "N" }')

check_invalid empty_input 1 'invalid course or candidate count' ''
check_invalid malformed_header 1 'invalid course or candidate count' 'x 0\n'
check_invalid extra_header_field 1 'invalid course or candidate count' '0 0 1\n'
check_invalid negative_count 1 'invalid course or candidate count' '-1 0\n'
check_invalid count_limit 1 'invalid course or candidate count' '1001 0\n'
check_invalid count_overflow 1 'invalid course or candidate count' '18446744073709551616 0\n'
check_invalid candidate_without_course 1 'invalid course or candidate count' '0 1\n'
check_invalid missing_course_name 2 'invalid course name' '1 0\n'
check_invalid blank_course_name 2 'invalid course name' '1 0\n\n1\n'
check_invalid whitespace_course_name 2 'invalid course name' '1 0\n   \n1\n'
check_invalid control_course_name 2 'invalid course name' '1 0\nA\tB\n1\n'
check_invalid long_course_name 2 'invalid course name' "1 0\n${long_name}\n1\n"
check_invalid negative_seats 3 'invalid seat count' '1 0\nA\n-1\n'
check_invalid overflowing_seats 3 'invalid seat count' '1 0\nA\n18446744073709551616\n'
check_invalid missing_seats 3 'invalid seat count' '1 0\nA\n'
check_invalid missing_candidate_name 6 'invalid candidate name' "$two_courses"
check_invalid blank_candidate_name 6 'invalid candidate name' "${two_courses}\n1 0 1\n"
check_invalid long_candidate_name 6 'invalid candidate name' "${two_courses}${long_name}\n1 0 1\n"
check_invalid missing_candidate_details 7 'invalid score or course preferences' "${two_courses}X\n"
check_invalid malformed_score 7 'invalid score or course preferences' "${two_courses}X\nx 0 1\n"
check_invalid negative_score 7 'invalid score or course preferences' "${two_courses}X\n-1 0 1\n"
check_invalid nonfinite_nan 7 'invalid score or course preferences' "${two_courses}X\nnan 0 1\n"
check_invalid nonfinite_infinity 7 'invalid score or course preferences' "${two_courses}X\ninf 0 1\n"
check_invalid overflowing_score 7 'invalid score or course preferences' "${two_courses}X\n1e9999 0 1\n"
check_invalid underflowing_score 7 'invalid score or course preferences' "${two_courses}X\n1e-9999 0 1\n"
check_invalid hexadecimal_score 7 'invalid score or course preferences' "${two_courses}X\n0x1p3 0 1\n"
check_invalid malformed_exponent 7 'invalid score or course preferences' "${two_courses}X\n1e+ 0 1\n"
check_invalid invalid_course_index 7 'invalid score or course preferences' "${two_courses}X\n1 0 2\n"
check_invalid negative_course_index 7 'invalid score or course preferences' "${two_courses}X\n1 -1 1\n"
check_invalid duplicate_choice 7 'invalid score or course preferences' "${two_courses}X\n1 0 0\n"
check_invalid extra_candidate_field 7 'invalid score or course preferences' "${two_courses}X\n1 0 1 extra\n"
check_invalid trailing_data 2 'unexpected trailing data' '0 0\nextra\n'
check_invalid trailing_nul 2 'unexpected trailing data' '0 0\n\000\n'
check_invalid embedded_nul 1 'invalid course or candidate count' '0\000 0\n'

check_valid no_final_newline "$fixtures/zero_seats.out" \
    '2 1\nHistory\n0\nGeography\n1\nErin\n42 0 1'
check_valid decimal_exponent "$fixtures/zero_seats.out" \
    '2 1\nHistory\n0\nGeography\n1\nErin\n42e0 0 1\n'
check_valid explicit_plus_signs "$fixtures/zero_seats.out" \
    '+2 +1\nHistory\n+0\nGeography\n+1\nErin\n+42e0 +0 +1\n'
check_valid trailing_whitespace "$fixtures/zero_courses.out" '0 0\n \n\t\n'

awk '{ printf "%s\r\n", $0 }' "$fixtures/basic.in" > "$temporary_dir/input"
"$binary" < "$temporary_dir/input" > "$temporary_dir/actual.out" \
    2> "$temporary_dir/actual.err"
diff -u "$fixtures/basic.out" "$temporary_dir/actual.out"
test ! -s "$temporary_dir/actual.err"
printf 'PASS crlf_input\n'

printf '1 0\n%s\n0\n' "$maximum_name" > "$temporary_dir/input"
"$binary" < "$temporary_dir/input" > "$temporary_dir/actual.out" \
    2> "$temporary_dir/actual.err"
printf '%s 0.00\nAdmitted\nWaiting list\n' "$maximum_name" \
    > "$temporary_dir/expected.out"
diff -u "$temporary_dir/expected.out" "$temporary_dir/actual.out"
test ! -s "$temporary_dir/actual.err"
printf 'PASS maximum_name_length\n'

printf '2 1\nA\n0\nB\n0\n%s\n0 0 1\n' "$maximum_name" > "$temporary_dir/input"
"$binary" < "$temporary_dir/input" > "$temporary_dir/actual.out" \
    2> "$temporary_dir/actual.err"
printf 'A 0.00\nAdmitted\nWaiting list\n%s 0.00\n\nB 0.00\nAdmitted\nWaiting list\n%s 0.00\n' \
    "$maximum_name" "$maximum_name" > "$temporary_dir/expected.out"
diff -u "$temporary_dir/expected.out" "$temporary_dir/actual.out"
test ! -s "$temporary_dir/actual.err"
printf 'PASS maximum_candidate_name_length\n'
