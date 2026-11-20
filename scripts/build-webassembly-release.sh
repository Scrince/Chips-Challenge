#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_VERSION="$(sed -n 's/^project(ChipsChallengeNative VERSION \([^ ]*\).*/\1/p' "${ROOT_DIR}/CMakeLists.txt")"
VERSION="${1:-${VERSION:-${PROJECT_VERSION}}}"
BUILD_DIR="${2:-${ROOT_DIR}/build/webassembly-release}"
case "${BUILD_DIR}" in
  /*) ;;
  *) BUILD_DIR="${ROOT_DIR}/${BUILD_DIR}" ;;
esac

if [[ ! "${VERSION}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Usage: $0 [version] [build-directory]" >&2
  exit 2
fi
if ! command -v emcmake >/dev/null 2>&1; then
  echo "Emscripten was not found. Install and activate emsdk, then retry." >&2
  exit 127
fi

RELEASE_DIR="${ROOT_DIR}/releases/${VERSION}"
PACKAGE_NAME="Chips-Challenge-${VERSION}-WebAssembly"
PACKAGE_DIR="${RELEASE_DIR}/webassembly/${PACKAGE_NAME}"
ARCHIVE="${RELEASE_DIR}/webassembly/${PACKAGE_NAME}.zip"
mkdir -p "${BUILD_DIR}" "${RELEASE_DIR}/webassembly"

FETCH_ARGS=()
for dependency in SDL3 SDL3_TTF; do
  case "${dependency}" in
    SDL3) source_dir="${ROOT_DIR}/build/webassembly/_deps/sdl3-src" ;;
    SDL3_TTF) source_dir="${ROOT_DIR}/build/webassembly/_deps/sdl3_ttf-src" ;;
  esac
  if [[ -f "${source_dir}/CMakeLists.txt" ]]; then
    FETCH_ARGS+=("-DFETCHCONTENT_SOURCE_DIR_${dependency}=${source_dir}")
  fi
done

emcmake cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  "${FETCH_ARGS[@]}"
if [[ -d "${BUILD_DIR}/_deps" ]]; then
  find "${BUILD_DIR}/_deps" -type f -name '._*' -delete
fi
emcmake cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  "${FETCH_ARGS[@]}"
if ! cmake --build "${BUILD_DIR}" --target chips-native --parallel \
    >"${BUILD_DIR}/build.log" 2>&1; then
  tail -n 80 "${BUILD_DIR}/build.log" >&2
  exit 1
fi
python3 "${ROOT_DIR}/tests/webassembly_build_test.py" "${BUILD_DIR}"
node --check "${ROOT_DIR}/web/midi-player.js"
node --check "${ROOT_DIR}/web/soundfont-midi.js"
node "${ROOT_DIR}/tests/webassembly_audio_test.js"

SDL3_SOURCE_DIR="${SDL3_SOURCE_DIR:-${ROOT_DIR}/build/webassembly/_deps/sdl3-src}"
SDL3_TTF_SOURCE_DIR="${SDL3_TTF_SOURCE_DIR:-${ROOT_DIR}/build/webassembly/_deps/sdl3_ttf-src}"
for license in \
  "${SDL3_SOURCE_DIR}/LICENSE.txt" \
  "${SDL3_TTF_SOURCE_DIR}/LICENSE.txt" \
  "${SDL3_TTF_SOURCE_DIR}/external/freetype/LICENSE.TXT"; do
  if [[ ! -f "${license}" ]]; then
    echo "Required third-party license is missing: ${license}" >&2
    exit 1
  fi
done

rm -rf "${PACKAGE_DIR}" "${ARCHIVE}"
mkdir -p "${PACKAGE_DIR}"
for artifact in chips-native.html chips-native.js chips-native.wasm chips-native.data midi-player.js soundfont-midi.js chips-help.html favicon.ico; do
  if [[ ! -s "${BUILD_DIR}/${artifact}" ]]; then
    echo "Missing WebAssembly release artifact: ${BUILD_DIR}/${artifact}" >&2
    exit 1
  fi
  cp "${BUILD_DIR}/${artifact}" "${PACKAGE_DIR}/${artifact}"
done
cp "${ROOT_DIR}/scripts/Launch-Chips-Challenge.cmd" "${PACKAGE_DIR}/Launch-Chips-Challenge.cmd"
cp "${ROOT_DIR}/scripts/Launch-Chips-Challenge.ps1" "${PACKAGE_DIR}/Launch-Chips-Challenge.ps1"
cp "${ROOT_DIR}/scripts/launch-webassembly-unix.sh" "${PACKAGE_DIR}/Launch-Chips-Challenge.sh"
cp "${ROOT_DIR}/scripts/launch-webassembly-unix.sh" "${PACKAGE_DIR}/Launch-Chips-Challenge.command"
chmod +x "${PACKAGE_DIR}/Launch-Chips-Challenge.sh" "${PACKAGE_DIR}/Launch-Chips-Challenge.command"
cp "${BUILD_DIR}/chips-native.html" "${PACKAGE_DIR}/index.html"
cp -R "${ROOT_DIR}/web/vendor" "${PACKAGE_DIR}/vendor"
mkdir -p "${PACKAGE_DIR}/soundfonts"
cp "${ROOT_DIR}/web/soundfonts/GeneralUser-GS.sf2" "${PACKAGE_DIR}/soundfonts/GeneralUser-GS.sf2"
cp "${ROOT_DIR}/web/soundfonts/LICENSE.txt" "${PACKAGE_DIR}/soundfonts/LICENSE.txt"
cp "${ROOT_DIR}/assets/ART.md" "${PACKAGE_DIR}/ART-CREDITS.md"
cp "${ROOT_DIR}/assets/AUDIO.md" "${PACKAGE_DIR}/AUDIO-CREDITS.md"
cp "${ROOT_DIR}/assets/fonts/OFL.txt" "${PACKAGE_DIR}/NotoSans-OFL.txt"
cp "${ROOT_DIR}/CHANGELOG.md" "${PACKAGE_DIR}/CHANGELOG.md"
printf '%s\n' "${VERSION}" >"${PACKAGE_DIR}/VERSION.txt"
cat >"${PACKAGE_DIR}/README.txt" <<EOF
Chip's Challenge ${VERSION} — WebAssembly

This is a static browser build. Extract the complete folder and deploy all of
its files together to a web server. Open index.html or chips-native.html over
HTTP or HTTPS; opening either page directly with file:// is not supported.

For local testing, run: python3 -m http.server 8000
Then open: http://localhost:8000/

The server must serve .wasm files as application/wasm for streaming startup.
The loader also includes a non-streaming fallback. Click or tap the game once
to enable browser audio. Progress is saved in browser IndexedDB for this site.
Custom .dat level packs are selected through the in-game Level menu.
The browser synthesizer loads its included GeneralUser GS sound bank the first
time background music starts; later launches can reuse the browser cache.

This WebAssembly package is unsigned. Its ZIP checksum is recorded in
SHA256SUMS.txt next to the archive.
EOF
{
  printf 'Third-party notices for Chip\x27s Challenge %s WebAssembly\n\n' "${VERSION}"
  printf 'SDL3 (zlib license)\n\n'
  cat "${SDL3_SOURCE_DIR}/LICENSE.txt"
  printf '\n\nSDL3_ttf license\n\n'
  cat "${SDL3_TTF_SOURCE_DIR}/LICENSE.txt"
  printf '\n\nFreeType license\n\n'
  cat "${SDL3_TTF_SOURCE_DIR}/external/freetype/LICENSE.TXT"
  printf '\n\nWeb MIDI synthesizer notices\n\n'
  cat "${ROOT_DIR}/web/vendor/THIRD-PARTY-NOTICES.txt"
  printf '\n\nGeneralUser GS sound bank license\n\n'
  cat "${ROOT_DIR}/web/soundfonts/LICENSE.txt"
} >"${PACKAGE_DIR}/ThirdPartyNotices.txt"

python3 - "${RELEASE_DIR}/webassembly" "${PACKAGE_NAME}" <<'PY'
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile
import sys

release_dir = Path(sys.argv[1])
package_name = sys.argv[2]
package_dir = release_dir / package_name
archive = release_dir / f"{package_name}.zip"
with ZipFile(archive, "w", compression=ZIP_DEFLATED, compresslevel=9) as output:
    for path in sorted(package_dir.rglob("*")):
        if path.is_file():
            output.write(path, path.relative_to(release_dir).as_posix())
with ZipFile(archive) as package:
    bad_member = package.testzip()
    if bad_member:
        raise SystemExit(f"ZIP integrity check failed at {bad_member}")
print(f"WebAssembly release package ready: {archive}")
PY
(cd "${RELEASE_DIR}/webassembly" && {
  shasum -a 256 "${PACKAGE_NAME}.zip" >SHA256SUMS.txt
  shasum -a 256 -c SHA256SUMS.txt
})
echo "WebAssembly ${VERSION} package ready at ${ARCHIVE}"
