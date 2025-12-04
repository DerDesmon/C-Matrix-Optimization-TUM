CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3 -msse3

# directories for source files and executables
INPUTS_DIR = dev/inputs
BUILD_DIR = bin
GENERATOR_DIST_DIR = dev/inputs/dist
CODE_OUTPUT_DIR = gen
SRC_DIR = src

TESTGEN_SRC_FILES = $(INPUTS_DIR)/test-generator.c
BENCHMARKGEN_SRC_FILES = $(INPUTS_DIR)/generators-main.c $(INPUTS_DIR)/benchmark-generator.c                   

SHARED_SRC = $(SRC_DIR)/io.c $(SRC_DIR)/matrix_utils.c $(SRC_DIR)/benchmark.c dev/benchmark_dev.c dev/test.c $(SRC_DIR)/matmul.c $(SRC_DIR)/matmul_caller.c include/ellpack.c
MAIN_SRC = $(SHARED_SRC) $(SRC_DIR)/main.c

TESTGEN_EXEC = $(BUILD_DIR)/test-generator
BENCHMARKGEN_EXEC = $(BUILD_DIR)/benchmark-generator
MAIN_EXEC = $(BUILD_DIR)/main


all: compile

all-generate: compile generate

generate: generate-test generate-benchmark clean-generators
compile: $(MAIN_EXEC)

generate-test: $(TESTGEN_EXEC)
	./$<

generate-benchmark: $(BENCHMARKGEN_EXEC)
	./$< 

run: $(MAIN_EXEC)
	./$<

$(TESTGEN_EXEC): $(TESTGEN_SRC_FILES)| $(BUILD_DIR) $(GENERATOR_DIST_DIR)
	$(CC) $(CFLAGS) -o $@ $(TESTGEN_SRC_FILES)

$(BENCHMARKGEN_EXEC): $(BENCHMARKGEN_SRC_FILES) | $(BUILD_DIR) $(GENERATOR_DIST_DIR)
	$(CC) $(CFLAGS) -o $@ $(BENCHMARKGEN_SRC_FILES)

$(MAIN_EXEC): $(MAIN_SRC) | $(BUILD_DIR) $(CODE_OUTPUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_SRC)

$(BUILD_DIR) $(GENERATOR_DIST_DIR) $(CODE_OUTPUT_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(GENERATOR_DIST_DIR) $(CODE_OUTPUT_DIR)

clean-generators:
	rm -f $(TESTGEN_EXEC) $(BENCHMARKGEN_EXEC)

.PHONY: all clean generate-test clean-generators
