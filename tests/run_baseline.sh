#!/bin/sh
set -eu

binary=$1
fixtures=$2
temporary_dir=$(mktemp -d)
trap 'rm -rf "$temporary_dir"' EXIT HUP INT TERM

for scenario in basic ties second_choice underfilled empty zero_courses zero_seats; do
    "$binary" < "$fixtures/$scenario.in" > "$temporary_dir/actual.out" \
        2> "$temporary_dir/actual.err"
    if [ -s "$temporary_dir/actual.err" ]; then
        printf 'Unexpected stderr in %s\n' "$scenario" >&2
        cat "$temporary_dir/actual.err" >&2
        exit 1
    fi
    diff -u "$fixtures/$scenario.out" "$temporary_dir/actual.out"
    printf 'PASS %s\n' "$scenario"
done
