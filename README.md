# Sparse Matrix Multiplication in ELLPACK (Column-Major)

Optimized C implementation of sparse matrix multiplication using the ELLPACK format with a cache-friendly row extraction strategy and optional SIMD acceleration. Built as a university project (TUM) and prepared for benchmarking, testing, and CLI usage.

## Highlights
- ELLPACK (column-major) representation with read-only and mutable views
- Row-Cache to mitigate column-major cache inefficiency
- Two core paths: baseline and SIMD-accelerated multiplication
- Benchmarks and randomized test generation included
- Clear data structures and Doxygen-style header docs

## Project Structure
- `Implementierung/src/`: core implementation (`matmul.c`, `matrix_utils.c`, `io.c`, `benchmark.c`, `main.c`)
- `Implementierung/include/`: public data structures (`ellpack.h`)
- `Implementierung/dev/`: generators and test/benchmark drivers
- `Projektbericht.md`: German project report (specs, environment, results)
- `Vortrag/`: presentation materials placeholder

## Build
Requires `gcc` (tested on Ubuntu 24.04, gcc 13.2).

```bash
cd Implementierung
make
```

The Makefile builds the CLI binary (typically `main` or similar). Check the produced artifacts in `Implementierung` after build.

## Release Build (Lean)
Use the minimal binary without dev-only generators/tests:

```bash
cd Implementierung
make release
./bin/main_release -a input_A.ellpack -b input_B.ellpack -o result.out -V 1 -B 50
```

This targets only the core sources (`src/*`, `include/ellpack.c`) and keeps optional benchmarking without the `dev/*` dependencies.

## Quick Start
- Build:
	```bash
	cd Implementierung
	make        # builds bin/main_release
	```
- Run multiplication:
	```bash
	./bin/main_release -a ../samples/input_A.ellpack -b ../samples/input_B.ellpack -o ../gen/result.out -V 1
	```
- Benchmark repetitions (optional):
	```bash
	./bin/main_release -a ../samples/input_A.ellpack -b ../samples/input_B.ellpack -o ../gen/result.out -V 1 -B 50
	```
- Sanity run:
	```bash
	./scripts/sanity.sh
	```
	Writes output to `gen/sanity_result.txt` and prints a success line.


CLI flags:
- `-a` path to matrix A (ELLPACK)
- `-b` path to matrix B (ELLPACK)
- `-o` output path (default: `gen/matrix.txt`)
- `-V` version: `0=auto`, `1=SIMD`, `2=no SIMD`, `3=unsorted`
- `-B` repetitions for benchmarking (e.g., `-B` or `-B5`)

## Run
The CLI reads two matrices, multiplies them, and optionally writes the result.

```bash
# Example usage (filenames illustrative)
./main input_A.ellpack input_B.ellpack result.out

# With benchmarking (if supported by CLI flags)
./main input_A.ellpack input_B.ellpack result.out --bench 50
```

File format: matrices are expected in the project’s ELLPACK layout as handled by `io.c` (`read_ellpack_matrix`/`write_ellpack_matrix`).

## API Overview
- `ELLPACKMatrix` / `const_ELLPACKMatrix`: ELLPACK data structures
- `result_mat`: dynamic column-wise accumulator for results
- `matmul_func`: function pointer type for multiplication
- `matr_mult_ellpack_main_simd(...)`: SIMD-accelerated core
- `matr_mult_ellpack_main_no_simd(...)`: baseline core
- `matr_mult_ellpack_unsorted(...)`: handles unsorted column indices
- `call_matmul(...)`: ties I/O + benchmarking + matmul implementation

Headers provide Doxygen-style documentation for all public types and functions.

## Benchmarking
Use `Implementierung/src/benchmark.c` or the CLI flag to repeat multiplication and measure performance. The environment used in our measurements:
- CPU: Ryzen 5 4500U (~1.4 GHz)
- RAM: 7.4 GB
- OS: Ubuntu 24.04 LTS, Kernel 6.8.0-38
- Compiler: gcc 13.2.0

Findings:
- SIMD yields significant speedups, increasing with matrix size
- Logic optimization (row-cache reuse) reduces unnecessary scans

## Testing
`Implementierung/dev/` contains generators for dense matrices and their products using a trivial reference multiplication. The automated test compares ELLPACK multiplication results against the dense reference.

```bash
cd Implementierung/dev
make
./generators-main    # produces sample inputs
./test               # runs validation against reference
```

## Contributing / Extending
- Add additional formats (CSR/COO) by implementing matching I/O + core kernels
- Extend SIMD paths to AVX/AVX2/NEON depending on target
- Integrate a proper CLI flag parser for configurable runs

## License
No explicit license provided; please contact the authors before reuse beyond evaluation or demonstration.
