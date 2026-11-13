#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="${1:-${ROOT_DIR}/dist-macos}"
OUTPUT_DIR="$(mkdir -p "${OUTPUT_DIR}" && cd "${OUTPUT_DIR}" && pwd)"
VERSION="${VERSION:-$(sed -n 's/^project(ChipsChallengeNative VERSION \([^ ]*\).*/\1/p' "${ROOT_DIR}/CMakeLists.txt") }"
VERSION="${VERSION// /}"
SDL3_SOURCE_DIR="${SDL3_SOURCE_DIR:-}"
if [[ -z "${SDL3_SOURCE_DIR}" ]]; then
  for candidate in "${ROOT_DIR}"/build-*/_deps/sdl3-src; do
    if [[ -f "${candidate}/CMakeLists.txt" ]]; then
      SDL3_SOURCE_DIR="${candidate}"
      break
    fi
  done
fi

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "This release script must run on macOS." >&2
  exit 1
fi

build_and_package() {
  local label="$1"
  local archs="$2"
  local build_dir="${OUTPUT_DIR}/build-${label}"
  local stage_dir="${OUTPUT_DIR}/stage-${label}"
  local app_dir="${OUTPUT_DIR}/Chips Challenge-${label}.app"
  local zip_path="${OUTPUT_DIR}/Chips-Challenge-${VERSION}-macOS-${label}.zip"

  cmake -S "${ROOT_DIR}" -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="${archs}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-11.0}" \
    ${SDL3_SOURCE_DIR:+-DFETCHCONTENT_SOURCE_DIR_SDL3="${SDL3_SOURCE_DIR}"}
  cmake --build "${build_dir}" --config Release --parallel

  rm -rf "${stage_dir}" "${app_dir}" "${zip_path}"
  cmake --install "${build_dir}" --config Release --prefix "${stage_dir}"

  mkdir -p "${app_dir}/Contents/MacOS" "${app_dir}/Contents/Resources"
  cp "${stage_dir}/chips-native" "${app_dir}/Contents/MacOS/chips-native"
  cp "${stage_dir}/libSDL3.0.dylib" "${app_dir}/Contents/MacOS/libSDL3.0.dylib"
  ln -s "libSDL3.0.dylib" "${app_dir}/Contents/MacOS/libSDL3.dylib"
  cp -R "${stage_dir}/assets" "${app_dir}/Contents/Resources/assets"
  cp -R "${stage_dir}/data" "${app_dir}/Contents/Resources/data"
  cat > "${app_dir}/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key><string>Chips Challenge</string>
  <key>CFBundleDisplayName</key><string>Chips Challenge</string>
  <key>CFBundleIdentifier</key><string>com.chipschallenge.native</string>
  <key>CFBundleVersion</key><string>${VERSION}</string>
  <key>CFBundleShortVersionString</key><string>${VERSION}</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleExecutable</key><string>chips-native</string>
  <key>LSMinimumSystemVersion</key><string>${MACOSX_DEPLOYMENT_TARGET:-11.0}</string>
  <key>NSHighResolutionCapable</key><true/>
</dict>
</plist>
PLIST

  ditto -c -k --sequesterRsrc --keepParent "${app_dir}" "${zip_path}"
  chmod 644 "${zip_path}"
  rm -rf "${stage_dir}" "${build_dir}"
  echo "Created ${zip_path}"
}

build_and_package arm64 arm64
build_and_package x86_64 x86_64
build_and_package universal "arm64;x86_64"
