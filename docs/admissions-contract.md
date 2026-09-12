# Admissions contract

This document defines the implemented behavior of University Admissions Simulator. The seven
`tests/fixtures/legacy/*.out` files capture actual output from the original program before any
source changes. Their Portuguese headings are historical evidence, not the target output language.
Current fixtures additionally cover duplicate candidate names, cascading removals, input limits,
and malformed input.

## Input

The program reads one simulation from standard input. There are no command-line arguments or input
files. Its line-oriented format is:

```text
<course_count> <candidate_count>
<course_0_name>
<course_0_seats>
...
<course_N_name>
<course_N_seats>
<candidate_0_name>
<candidate_0_score> <candidate_0_first_choice> <candidate_0_second_choice>
...
```

Names occupy a line and may contain spaces. A course or candidate name must contain 1–99 bytes,
excluding the line ending, have at least one non-space character, and contain no ASCII control
characters. Counts and seats are decimal integers; counts are nonnegative and seats
are nonnegative. The program supports at most 1,000 courses and 100,000 candidates, subject to
available memory. Zero courses are allowed only when there are zero candidates. Each candidate's
two course indices are zero-based, valid, and distinct. Scores are finite, nonnegative decimal
numbers representable by the program's numeric type. Candidate names do not have to be unique;
input position is their identity. Course names may also repeat. Decimal scores may use an exponent,
such as `4.2e1`; values that overflow or underflow to zero are invalid.

Blank lines are not allowed in place of names. Spaces separate numeric fields. CRLF line endings
and a final line without a newline are accepted. Unexpected trailing data is invalid.

For example, `tests/fixtures/basic.in` contains two courses and four candidates.

## Allocation and ordering

Each course ranks its applicants by descending score. At an equal score, a first-choice applicant
precedes a second-choice applicant; candidates within the same preference category retain input
order. The top `seats` applicants are admitted. A candidate admitted to their first choice is
removed from their second-choice list. Resolve removals and promotions until no list changes.
Candidates admitted only to a second choice remain eligible for their first-choice waiting list.
No candidate may be admitted to two courses at once.

The cutoff is the score of the candidate in the final seat when every seat is filled. It is `0.00`
when a course is underfilled, empty, or has zero seats, matching the original program. All applicants
outside the admitted group appear on the waiting list in ranking order. A zero-seat course therefore
has all its applicants on the waiting list.

## Output and errors

For each course, stdout contains its name and cutoff on the first line, then `Admitted`, admitted
candidates, `Waiting list`, and waiting candidates. Scores have two decimal places. There is one
blank line between courses and a final newline; there is no heading or blank line before the first
course. Zero courses produce empty stdout.

```text
Computer Science 80.00
Admitted
Alice 90.00
Carol 80.00
Waiting list
Dave 75.00

Medicine 85.00
Admitted
Bob 85.00
Waiting list
Dave 75.00
```

Invalid or incomplete input must produce no report on stdout, an English diagnostic on stderr,
and a nonzero exit status. Allocation failures also return a nonzero status after cleanup. Error
messages must identify the failing input category; exact wording is not part of the public format.

## Legacy observations and intentional changes

- The original program prints `Classificados` and `Lista de espera`; target headings are English.
- The original list inserts first-choice candidates before second-choice candidates on a score
  tie, and preserves input order within each category. The target makes this rule explicit.
- The original program uses names to match candidate records, so duplicate names can change the
  wrong application. The target accepts duplicate names using stable candidate IDs.
- The original program has unsafe behavior for repeated preferences and malformed input. The
  current program rejects repeated preferences and malformed input instead of preserving that
  behavior.
- The original program prints `0.00` for underfilled cutoffs. The target preserves this rule.
- The original report does not consistently separate empty courses. The target uses one blank line
  between all adjacent course reports.
- The original report hides applicants on a zero-seat course. The target lists them as waiting.
