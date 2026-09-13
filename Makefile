CC ?= cc
COVERAGE_CC ?= gcc
ANALYZER_CC ?= gcc
GCOV ?= gcov
CLANG_FORMAT ?= clang-format
ASAN_OPTIONS ?= detect_leaks=0
CPPFLAGS ?=
CFLAGS ?= -O2
LDFLAGS ?=
LDLIBS ?=

STANDARD_FLAGS := -std=c17
WARNING_FLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror
DEPENDENCY_FLAGS := -MMD -MP
SANITIZER_FLAGS := -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined
COVERAGE_FLAGS := -g -O0 --coverage
ANALYZER_FLAGS := -g -O0 -fanalyzer

TARGET := university-admissions-simulator
MODULES := main application admissions_io admissions_model admissions_ranking \
	admissions_allocation admissions_report
SOURCES := $(addprefix src/,$(addsuffix .c,$(MODULES)))
HEADERS := $(wildcard include/*.h)
TEST_SOURCES := tests/test_modules.c tests/test_exhaustive.c tests/test_faults.c
FORMAT_SOURCES := $(SOURCES) $(TEST_SOURCES) $(HEADERS)
FAULT_LINK_FLAGS := -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free
TEST_SCRIPT := tests/run_baseline.sh
VALIDATION_SCRIPT := tests/run_validation.sh
LIMITS_SCRIPT := tests/run_limits.sh
FIXTURES := tests/fixtures

RELEASE_DIR := build/release/objects
SANITIZER_DIR := build/sanitize
COVERAGE_DIR := build/coverage
ANALYZER_DIR := build/analyzer/objects
TEST_DIR := build/tests
RELEASE_OBJECTS := $(addprefix $(RELEASE_DIR)/,$(addsuffix .o,$(MODULES)))
SANITIZER_OBJECTS := $(addprefix $(SANITIZER_DIR)/objects/,$(addsuffix .o,$(MODULES)))
COVERAGE_OBJECTS := $(addprefix $(COVERAGE_DIR)/objects/,$(addsuffix .o,$(MODULES)))
ANALYZER_OBJECTS := $(addprefix $(ANALYZER_DIR)/,$(addsuffix .o,$(MODULES)))
CORE_MODULES := admissions_io admissions_model admissions_ranking admissions_allocation admissions_report
ALGORITHM_MODULES := admissions_model admissions_ranking admissions_allocation
PARSER_MODULES := admissions_io admissions_model admissions_ranking
RELEASE_CORE_OBJECTS := $(addprefix $(RELEASE_DIR)/,$(addsuffix .o,$(CORE_MODULES)))
SANITIZER_CORE_OBJECTS := $(addprefix $(SANITIZER_DIR)/objects/,$(addsuffix .o,$(CORE_MODULES)))
COVERAGE_CORE_OBJECTS := $(addprefix $(COVERAGE_DIR)/objects/,$(addsuffix .o,$(CORE_MODULES)))
RELEASE_ALGORITHM_OBJECTS := $(addprefix $(RELEASE_DIR)/,$(addsuffix .o,$(ALGORITHM_MODULES)))
SANITIZER_ALGORITHM_OBJECTS := $(addprefix $(SANITIZER_DIR)/objects/,$(addsuffix .o,$(ALGORITHM_MODULES)))
COVERAGE_ALGORITHM_OBJECTS := $(addprefix $(COVERAGE_DIR)/objects/,$(addsuffix .o,$(ALGORITHM_MODULES)))
RELEASE_PARSER_OBJECTS := $(addprefix $(RELEASE_DIR)/,$(addsuffix .o,$(PARSER_MODULES)))
SANITIZER_PARSER_OBJECTS := $(addprefix $(SANITIZER_DIR)/objects/,$(addsuffix .o,$(PARSER_MODULES)))
COVERAGE_PARSER_OBJECTS := $(addprefix $(COVERAGE_DIR)/objects/,$(addsuffix .o,$(PARSER_MODULES)))
RELEASE_TEST_OBJECTS := $(addprefix $(RELEASE_DIR)/tests/,$(addsuffix .o,modules exhaustive faults))
SANITIZER_TEST_OBJECTS := $(addprefix $(SANITIZER_DIR)/objects/tests/,$(addsuffix .o,modules exhaustive faults))
COVERAGE_TEST_OBJECTS := $(addprefix $(COVERAGE_DIR)/objects/tests/,$(addsuffix .o,modules exhaustive faults))
DEPENDENCIES := $(RELEASE_OBJECTS:.o=.d) $(SANITIZER_OBJECTS:.o=.d) \
	$(COVERAGE_OBJECTS:.o=.d) $(ANALYZER_OBJECTS:.o=.d) \
	$(RELEASE_TEST_OBJECTS:.o=.d) $(SANITIZER_TEST_OBJECTS:.o=.d) \
	$(COVERAGE_TEST_OBJECTS:.o=.d)

.PHONY: all test sanitize coverage analyze format check-format clean

all: $(TARGET)

$(RELEASE_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(RELEASE_DIR)/tests/%.o: tests/test_%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(SANITIZER_DIR)/objects/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(SANITIZER_DIR)/objects/tests/%.o: tests/test_%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(COVERAGE_DIR)/objects/%.o: src/%.c
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(COVERAGE_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(COVERAGE_DIR)/objects/tests/%.o: tests/test_%.c
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(COVERAGE_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(ANALYZER_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(ANALYZER_CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(ANALYZER_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(TARGET): $(RELEASE_OBJECTS)
	$(CC) $(RELEASE_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_DIR)/$(TARGET): $(SANITIZER_OBJECTS)
	$(CC) $(SANITIZER_FLAGS) $(SANITIZER_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_DIR)/$(TARGET): $(COVERAGE_OBJECTS)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $(COVERAGE_OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(TEST_DIR)/module-tests: $(RELEASE_DIR)/tests/modules.o $(RELEASE_CORE_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(TEST_DIR)/exhaustive-tests: $(RELEASE_DIR)/tests/exhaustive.o $(RELEASE_ALGORITHM_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(TEST_DIR)/fault-tests: $(RELEASE_DIR)/tests/faults.o $(RELEASE_PARSER_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $^ $(FAULT_LINK_FLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_DIR)/tests/module-tests: $(SANITIZER_DIR)/objects/tests/modules.o $(SANITIZER_CORE_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_DIR)/tests/exhaustive-tests: $(SANITIZER_DIR)/objects/tests/exhaustive.o $(SANITIZER_ALGORITHM_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SANITIZER_DIR)/tests/fault-tests: $(SANITIZER_DIR)/objects/tests/faults.o $(SANITIZER_PARSER_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(SANITIZER_FLAGS) $^ $(FAULT_LINK_FLAGS) $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_DIR)/tests/module-tests: $(COVERAGE_DIR)/objects/tests/modules.o $(COVERAGE_CORE_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_DIR)/tests/exhaustive-tests: $(COVERAGE_DIR)/objects/tests/exhaustive.o $(COVERAGE_ALGORITHM_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(COVERAGE_DIR)/tests/fault-tests: $(COVERAGE_DIR)/objects/tests/faults.o $(COVERAGE_PARSER_OBJECTS)
	@mkdir -p $(@D)
	$(COVERAGE_CC) $(COVERAGE_FLAGS) $^ $(FAULT_LINK_FLAGS) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TARGET) $(TEST_DIR)/module-tests $(TEST_DIR)/exhaustive-tests $(TEST_DIR)/fault-tests
	$(TEST_DIR)/module-tests
	$(TEST_DIR)/exhaustive-tests
	$(TEST_DIR)/fault-tests
	$(TEST_SCRIPT) ./$(TARGET) $(FIXTURES)
	$(VALIDATION_SCRIPT) ./$(TARGET) $(FIXTURES)
	$(LIMITS_SCRIPT) ./$(TARGET)

sanitize: $(SANITIZER_DIR)/$(TARGET) $(SANITIZER_DIR)/tests/module-tests \
	$(SANITIZER_DIR)/tests/exhaustive-tests $(SANITIZER_DIR)/tests/fault-tests
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(SANITIZER_DIR)/tests/module-tests
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(SANITIZER_DIR)/tests/exhaustive-tests
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(SANITIZER_DIR)/tests/fault-tests
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(TEST_SCRIPT) ./$(SANITIZER_DIR)/$(TARGET) $(FIXTURES)
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(VALIDATION_SCRIPT) ./$(SANITIZER_DIR)/$(TARGET) $(FIXTURES)
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(LIMITS_SCRIPT) ./$(SANITIZER_DIR)/$(TARGET)

coverage: $(COVERAGE_DIR)/$(TARGET) $(COVERAGE_DIR)/tests/module-tests \
	$(COVERAGE_DIR)/tests/exhaustive-tests $(COVERAGE_DIR)/tests/fault-tests
	@rm -f $(COVERAGE_OBJECTS:.o=.gcda) $(COVERAGE_TEST_OBJECTS:.o=.gcda)
	$(COVERAGE_DIR)/tests/module-tests
	$(COVERAGE_DIR)/tests/exhaustive-tests
	$(COVERAGE_DIR)/tests/fault-tests
	$(TEST_SCRIPT) ./$(COVERAGE_DIR)/$(TARGET) $(FIXTURES)
	$(VALIDATION_SCRIPT) ./$(COVERAGE_DIR)/$(TARGET) $(FIXTURES)
	$(LIMITS_SCRIPT) ./$(COVERAGE_DIR)/$(TARGET)
	$(GCOV) -n -b -c $(COVERAGE_OBJECTS:.o=.gcno)

analyze: $(ANALYZER_OBJECTS)

format:
	$(CLANG_FORMAT) -i $(FORMAT_SOURCES)

check-format:
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_SOURCES)

clean:
	rm -rf build $(TARGET)

-include $(DEPENDENCIES)
