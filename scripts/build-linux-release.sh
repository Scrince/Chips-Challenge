#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/linux"
STAGE_DIR="${BUILD_DIR}/stage"
PACKAGE_DIR="${BUILD_DIR}/packages"
VERSION="${VERSION:-$(sed -n 's/^project(ChipsChallengeNative VERSION \([^ ]*\).*/\1/p' "${ROOT_DIR}/CMakeLists.txt")}"
ARCH="$(uname -m)"

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "This release script must run on Linux." >&2
  exit 1
fi
if [[ -z "${VERSION}" ]]; then
  echo "Could not determine the project version from CMakeLists.txt." >&2
  exit 1
fi

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --config Release --parallel
ctest --test-dir "${BUILD_DIR}" --build-config Release --output-on-failure

rm -rf "${STAGE_DIR}"
cmake --install "${BUILD_DIR}" --config Release --prefix "${STAGE_DIR}"
bash "${ROOT_DIR}/scripts/verify-no-cclp2-assets.sh" "${STAGE_DIR}"
bash "${ROOT_DIR}/scripts/verify-audio-assets.sh" \
  "${ROOT_DIR}/assets/audio" "${STAGE_DIR}/assets/audio"
"${STAGE_DIR}/chips-native" --validate "${STAGE_DIR}/data/CHIPS.DAT"
"${STAGE_DIR}/chips-native" --validate-atlas "${STAGE_DIR}/assets/art/tile-atlas.png"
"${STAGE_DIR}/chips-native" --validate-icon "${STAGE_DIR}/assets/art/app-icon-polished.png"
"${STAGE_DIR}/chips-native" --validate-digits "${STAGE_DIR}/assets/art/digital-digits.png"

mkdir -p "${PACKAGE_DIR}"
ARCHIVE="${PACKAGE_DIR}/Chips-Challenge-${VERSION}-Linux-${ARCH}.tar.gz"
rm -f "${ARCHIVE}"
tar --exclude='._*' --exclude='*/._*' -C "${STAGE_DIR}" -czf "${ARCHIVE}" .
echo "Linux release package ready: ${ARCHIVE}"
