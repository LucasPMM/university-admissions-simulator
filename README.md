# University Admissions Simulator

A C17 command-line program that ranks candidates for university courses using two course
preferences. The modernization is in progress on `refactor/modernize-mini-sisu`.

Build with `make` and run an example with:

```sh
./university-admissions-simulator < examples/sample.in
```

Run the current valid-input characterization tests with `make test`. The complete input and output
contract, including rules still scheduled for hardening, is in
[`docs/admissions-contract.md`](docs/admissions-contract.md). The final README rewrite is planned
for Phase 5.
