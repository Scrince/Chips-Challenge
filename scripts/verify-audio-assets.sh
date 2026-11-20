#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -ne 2 ]]; then
  echo "Usage: $0 <source-audio-directory> <packaged-audio-directory>" >&2
  exit 2
fi

SOURCE_DIR="$(cd "$1" && pwd)"
PACKAGE_DIR="$(cd "$2" && pwd)"
count=0
while IFS= read -r -d '' source_file; do
  relative_path="${source_file#"${SOURCE_DIR}"/}"
  packaged_file="${PACKAGE_DIR}/${relative_path}"
  if [[ ! -f "${packaged_file}" ]]; then
    echo "Packaged audio file is missing: ${relative_path}" >&2
    exit 1
  fi
  if ! cmp -s "${source_file}" "${packaged_file}"; then
    echo "Packaged audio file differs from its source: ${relative_path}" >&2
    exit 1
  fi
  count=$((count + 1))
done < <(find "${SOURCE_DIR}" -type f ! -name '._*' -print0)

if [[ "${count}" -eq 0 ]]; then
  echo "No audio files were found in ${SOURCE_DIR}." >&2
  exit 1
fi

echo "Verified ${count} packaged audio files."
