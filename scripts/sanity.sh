#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$repo_root/Implementierung"
make -j"$(nproc)" > /dev/null
out_file="$repo_root/gen/sanity_result.txt"
mkdir -p "$repo_root/gen"
./bin/main_release -a "$repo_root/samples/input_A.ellpack" -b "$repo_root/samples/input_B.ellpack" -o "$out_file" -V 1 || {
  echo "Sanity run failed" >&2
  exit 1
}
if [[ -s "$out_file" ]]; then
  echo "Sanity OK: wrote result to $out_file"
else
  echo "Sanity failed: result file empty" >&2
  exit 1
}
