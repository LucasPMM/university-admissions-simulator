#!/bin/sh
set -eu

binary=$1
temporary_dir=$(mktemp -d)
trap 'rm -rf "$temporary_dir"' EXIT HUP INT TERM

# Distribute candidates across courses so this exercises the documented limits without
# turning the ranking check into an unrelated quadratic stress benchmark.
awk 'BEGIN {
    print "1000 100000"
    for (course = 0; course < 1000; ++course) {
        print "Course " course
        print 0
    }
    for (candidate = 0; candidate < 100000; ++candidate) {
        print "Candidate " candidate
        print candidate % 3, candidate % 1000, (candidate + 1) % 1000
    }
}' > "$temporary_dir/input"

"$binary" < "$temporary_dir/input" > "$temporary_dir/output" \
    2> "$temporary_dir/error"
test ! -s "$temporary_dir/error"
test "$(wc -l < "$temporary_dir/output")" -eq 203999
test "$(grep -c '^Admitted$' "$temporary_dir/output")" -eq 1000
test "$(grep -c '^Waiting list$' "$temporary_dir/output")" -eq 1000
printf 'PASS maximum_supported_counts\n'
