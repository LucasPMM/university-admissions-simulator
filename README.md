# University Admissions Simulator

A C17 command-line program that assigns applicants to university courses using a score and two
ranked preferences. It reads one simulation from standard input and prints each course's admitted
applicants, waiting list, and cutoff score.

## Build and run

The program requires a C17 compiler and GNU Make. The full test suite also uses a POSIX shell,
standard command-line tools, and a GNU-compatible linker for allocation-failure tests.

```sh
make
./university-admissions-simulator < examples/sample.in
```

The example prints:

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

## Input format

Supply one line with the course and candidate counts, then two lines per course (name and seat
count), then two lines per candidate (name and details):

```text
<course_count> <candidate_count>
<course_0_name>
<course_0_seats>
...
<candidate_0_name>
<candidate_0_score> <candidate_0_first_choice> <candidate_0_second_choice>
...
```

- Counts and seats are nonnegative decimal integers. The limits are 1,000 courses and 100,000
  candidates, subject to available memory. Zero courses require zero candidates.
- Names may contain spaces, must be 1–99 bytes long with at least one non-space character, and
  cannot contain ASCII control characters. Names need not be unique.
- Scores are finite, nonnegative decimal numbers, optionally in exponent notation. Values that
  overflow or underflow to zero in C `float` are rejected. Single-precision scores preserve the
  original program's ranking and two-decimal rounding. The two preferences are distinct,
  zero-based course indices.
- Each name occupies its own line. Numeric fields are separated by whitespace. CRLF endings and
  a final line without a newline are accepted. Only whitespace may follow the last candidate.

Invalid or incomplete input produces an English diagnostic on standard error, no report on
standard output, and a nonzero exit status. The program also exits nonzero on memory or output
errors.

## Admission rules

Each course ranks applicants by descending score. At the same score, applicants who chose the
course first rank ahead of those who chose it second; input order breaks any remaining tie. The
highest-ranked applicants fill the available seats. An applicant admitted to their first choice
is removed from their second-choice list, which may promote someone else. The program repeats
these removals and promotions until the result is stable. An applicant admitted only to their
second choice remains on their first choice's waiting list. No applicant can hold two admissions.

Every course report lists admitted applicants, then its remaining applicants in waiting-list
order. Scores are printed to two decimal places. The cutoff is the score of the last admitted
applicant when every seat is filled; it is `0.00` for an underfilled or zero-seat course. Reports
are separated by one blank line. See [the admissions contract](docs/admissions-contract.md) for
the complete behavior specification and compatibility notes.

## Development

| Command | Purpose |
| --- | --- |
| `make test` | Run module, exhaustive, fault-injection, CLI, validation, and size-limit tests. |
| `make sanitize` | Run the same tests with AddressSanitizer and UndefinedBehaviorSanitizer. |
| `make coverage` | Run instrumented tests and print GCC `gcov` line and branch coverage. |
| `make analyze` | Compile the source with GCC's static analyzer. |
| `make check-format` | Check C source and headers with `clang-format`. |
| `make format` | Format C source and headers with `clang-format`. |
| `make clean` | Remove generated files. |

Generated objects and reports live under `build/`. To enable leak detection where the host
supports LeakSanitizer, run `make sanitize ASAN_OPTIONS=detect_leaks=1`. The
[CI workflow](.github/workflows/ci.yml) is configured to run tests with GCC and Clang, sanitizers
with leak detection, and formatting, static-analysis, and coverage checks on Ubuntu.

The code is divided into `src/` implementation modules and `include/` interfaces. `examples/`
contains runnable input, `tests/` contains direct module tests and CLI fixtures, and `docs/`
contains the detailed admissions contract.
