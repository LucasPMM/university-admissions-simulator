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
TEST_SCRIPT := tests/run_baseline.sh
VALIDATION_SCRIPT := tests/run_validation.sh
FIXTURES := tests/fixtures

RELEASE_DIR := build/release/objects
SANITIZER_DIR := build/sanitize
COVERAGE_DIR := build/coverage
ANALYZER_DIR := build/analyzer/objects
RELEASE_OBJECTS := $(addprefix $(RELEASE_DIR)/,$(addsuffix .o,$(MODULES)))
SANITIZER_OBJECTS := $(addprefix $(SANITIZER_DIR)/objects/,$(addsuffix .o,$(MODULES)))
COVERAGE_OBJECTS := $(addprefix $(COVERAGE_DIR)/objects/,$(addsuffix .o,$(MODULES)))
ANALYZER_OBJECTS := $(addprefix $(ANALYZER_DIR)/,$(addsuffix .o,$(MODULES)))
DEPENDENCIES := $(RELEASE_OBJECTS:.o=.d) $(SANITIZER_OBJECTS:.o=.d) \
	$(COVERAGE_OBJECTS:.o=.d) $(ANALYZER_OBJECTS:.o=.d)

.PHONY: all test sanitize coverage analyze format check-format clean

all: $(TARGET)

$(RELEASE_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(CFLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(SANITIZER_DIR)/objects/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iinclude $(STANDARD_FLAGS) $(WARNING_FLAGS) $(SANITIZER_FLAGS) \
		$(DEPENDENCY_FLAGS) -c $< -o $@

$(COVERAGE_DIR)/objects/%.o: src/%.c
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

test: $(TARGET)
	$(TEST_SCRIPT) ./$(TARGET) $(FIXTURES)
	$(VALIDATION_SCRIPT) ./$(TARGET) $(FIXTURES)

sanitize: $(SANITIZER_DIR)/$(TARGET)
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(TEST_SCRIPT) ./$(SANITIZER_DIR)/$(TARGET) $(FIXTURES)
	ASAN_OPTIONS=$(ASAN_OPTIONS) $(VALIDATION_SCRIPT) ./$(SANITIZER_DIR)/$(TARGET) $(FIXTURES)

coverage: $(COVERAGE_DIR)/$(TARGET)
	@rm -f $(COVERAGE_OBJECTS:.o=.gcda)
	$(TEST_SCRIPT) ./$(COVERAGE_DIR)/$(TARGET) $(FIXTURES)
	$(VALIDATION_SCRIPT) ./$(COVERAGE_DIR)/$(TARGET) $(FIXTURES)
	$(GCOV) -n -b -c $(COVERAGE_OBJECTS:.o=.gcno)

analyze: $(ANALYZER_OBJECTS)

format:
	$(CLANG_FORMAT) -i $(SOURCES) $(HEADERS)

check-format:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(HEADERS)

clean:
	rm -rf build $(TARGET)

-include $(DEPENDENCIES)
