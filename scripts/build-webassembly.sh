#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${1:-${repo_root}/build/webassembly}"
case "${build_dir}" in
    /*) ;;
    *) build_dir="${repo_root}/${build_dir}" ;;
esac

if ! command -v emcmake >/dev/null 2>&1; then
    echo "Emscripten was not found. Install and activate emsdk, then retry." >&2
    echo "See https://emscripten.org/docs/getting_started/downloads.html" >&2
    exit 127
fi

emcmake cmake -S "${repo_root}" -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# AppleDouble resource-fork files can appear beside fetched sources on
# macOS-mounted volumes. SDL's source globs otherwise mistake them for C/C++.
find "${build_dir}/_deps" -type f -name '._*' -delete
emcmake cmake -S "${repo_root}" -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if ! cmake --build "${build_dir}" --target chips-native --parallel \
    >"${build_dir}/build.log" 2>&1; then
    tail -n 80 "${build_dir}/build.log" >&2
    exit 1
fi
python3 "${repo_root}/tests/webassembly_build_test.py" "${build_dir}"
node --check "${repo_root}/web/midi-player.js"
node "${repo_root}/tests/webassembly_audio_test.js"

echo "WebAssembly development build is ready at ${build_dir}/chips-native.html"
echo "Serve it over HTTP; opening the HTML directly from disk is not supported."
