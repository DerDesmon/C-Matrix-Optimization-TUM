CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3 -msse3

# directories for source files and executables
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3 -msse3

# directories for source files and executables
BUILD_DIR = bin
SRC_DIR = src

# Minimal release build without dev helpers and generator code
RELEASE_SRC = $(SRC_DIR)/io.c $(SRC_DIR)/matrix_utils.c $(SRC_DIR)/benchmark.c $(SRC_DIR)/matmul.c $(SRC_DIR)/matmul_caller.c include/ellpack.c $(SRC_DIR)/main_release.c
MAIN_RELEASE_EXEC = $(BUILD_DIR)/main_release

# Default target: lean release build
all: release

release: $(MAIN_RELEASE_EXEC)

run: $(MAIN_RELEASE_EXEC)
	@if [ ! -f ../samples/input_A.ellpack ] || [ ! -f ../samples/input_B.ellpack ]; then \
		printf "Samples missing. Run scripts/sanity.sh or provide -a/-b paths.\n"; \
		exit 1; \
	fi
	./$< -a ../samples/input_A.ellpack -b ../samples/input_B.ellpack -o ../gen/matrix.txt -V 1

$(MAIN_RELEASE_EXEC): $(RELEASE_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(RELEASE_SRC)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean release run format

format:
	clang-format -i src/*.c src/*.h include/*.c include/*.h || true
