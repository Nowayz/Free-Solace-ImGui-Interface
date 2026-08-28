#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repository_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repository_root}/build/web"
temporary_dir="${build_dir}/tmp"

mkdir -p "${temporary_dir}"
export TMPDIR="${temporary_dir}"

if ! command -v emcmake >/dev/null 2>&1; then
    echo "emcmake was not found. Activate an Emscripten SDK before building." >&2
    exit 1
fi

if command -v cmake >/dev/null 2>&1; then
    emcmake cmake -S "${repository_root}" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Release
    cmake --build "${build_dir}" --parallel
else
    emmake make -C "${repository_root}" -f Makefile.web -j"$(getconf _NPROCESSORS_ONLN)"
fi

echo "WebGL2 SPA built at ${build_dir}/dist/index.html"
