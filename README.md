# Sparse Matrix Multiplication in ELLPACK (Column-Major)

![CI](https://github.com/DerDesmon/C-Matrix-Optimization-TUM/actions/workflows/ci.yml/badge.svg)

Optimized C implementation of sparse matrix multiplication using the ELLPACK format with a cache-friendly row extraction strategy and optional SIMD acceleration. Built as a university project (TUM). Prepared with a lean CLI and a sanity check for easy demonstration.

## Project Structure
- `Implementierung/src/`: core implementation (`matmul.c`, `matrix_utils.c`, `io.c`, `benchmark.c`, `main_release.c`)
- `Implementierung/include/`: public data structures (`ellpack.h`)
- `samples/`: minimal ELLPACK inputs for quick testing
- `scripts/`: `sanity.sh` to build and run a minimal validation
- `Vortrag/`: presentation materials (optional)

## Build
Requires `gcc` (tested on Ubuntu 24.04, gcc 13.2).

```bash
cd Implementierung
make
```

The Makefile builds the lean CLI binary `bin/main_release`. Check the produced artifacts in `Implementierung` after build.

## Release Build (Lean)

```bash
cd Implementierung
make release
./bin/main_release -a ../samples/input_A.ellpack -b ../samples/input_B.ellpack -o ../gen/result.out -V 1 -B 50
```

This targets only the core sources (`src/*`, `include/ellpack.c`) and keeps optional benchmarking. Legacy `dev/*` and an old project report were removed to streamline the repo.

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

## Input Format (ELLPACK)
- Column-major sparse format. Each column stores up to `nr_ellpack_elts` pairs of `(row_index, value)` and a per-column count.
- Files must follow the exact layout parsed by `Implementierung/src/io.c`. The bundled `samples/` are known-good examples.
- Tip: Use the provided samples first; if you craft your own files, mirror the same structure strictly to avoid parse errors (e.g., “negative dimension”).

Example (3x3, `nr_ellpack_elts=1`):
```
3,3,1
1.0,3.0,4.0
0,1,0
```

## Tests & Generators
- Legacy dev-only test/generator code was removed (`Implementierung/dev/*`) to keep the repository clean and minimal.
- Use `scripts/sanity.sh` for lightweight validation with bundled samples.
- If full test/generator tooling is needed, it can be restored in a separate branch without changing core logic.

## Authors
- Author: DerDesmon (repo owner)
- Collaborators: Artem Lomov and one additional teammate

## API Overview (brief)
- `ELLPACKMatrix` / `const_ELLPACKMatrix`: ELLPACK data structures
- `result_mat`: dynamic column-wise accumulator for results
- `matmul_func`: function pointer type for multiplication
- `matr_mult_ellpack_main_simd(...)`: SIMD-accelerated core
- `matr_mult_ellpack_main_no_simd(...)`: baseline core
- `matr_mult_ellpack_unsorted(...)`: handles unsorted column indices
- `call_matmul(...)`: ties I/O + benchmarking + matmul implementation
Headers include Doxygen-style documentation for public types/functions.

## Benchmarking
Use `Implementierung/src/benchmark.c` or the `-B` CLI flag to repeat multiplication and measure performance. For visual benchmark diagrams, see `Vortrag/Vortrag.pdf`.

## Testing
Use `scripts/sanity.sh` to validate build and a minimal multiplication run with bundled samples. To verify numerical correctness against a dense reference without NumPy, run:
```bash
python3 scripts/check_result.py samples/input_A5.ellpack samples/input_B5.ellpack gen/out_5x5.txt
```

## Contributing / Extending
- Add additional formats (CSR/COO) by implementing matching I/O + core kernels
- Extend SIMD paths to AVX/AVX2/NEON depending on target
- Integrate a proper CLI flag parser for configurable runs

## License
No explicit license provided; please contact the authors before reuse beyond evaluation or demonstration.
