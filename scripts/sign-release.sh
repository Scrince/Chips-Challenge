#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GPG_BIN="${GPG_BIN:-gpg}"
VERSION="${1:-}"
SOURCE_GNUPG_HOME="${GNUPGHOME:-${ROOT_DIR}/.gnupg-release}"
PUBKEY_SOURCE="${ROOT_DIR}/docs/ChipsChallenge_Release_Signing_2026_pubkey.asc"

if [[ ! "${VERSION}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Usage: $0 <version> (for example: $0 1.0.2)" >&2
  exit 2
fi

RELEASE_DIR="${ROOT_DIR}/releases/${VERSION}"
MANIFEST="${RELEASE_DIR}/SHA256SUMS"
MANIFEST_SIGNATURE="${MANIFEST}.asc"
PUBLIC_KEY="${RELEASE_DIR}/ReleaseSigningPublicKey.asc"

if [[ ! -d "${SOURCE_GNUPG_HOME}" ]]; then
  echo "Release keyring not found: ${SOURCE_GNUPG_HOME}" >&2
  echo "Initialize it first with scripts/init-release-key.sh" >&2
  exit 1
fi
if [[ ! -d "${RELEASE_DIR}" ]]; then
  echo "Release folder not found: ${RELEASE_DIR}" >&2
  exit 1
fi

shopt -s nullglob
artifacts=()
for platform_dir in "${RELEASE_DIR}"/*/; do
  for artifact in "${platform_dir}"*; do
    [[ -f "${artifact}" ]] || continue
    case "${artifact}" in
      *.zip|*.tar.gz|*.deb|*.rpm|*.dmg|*.exe|*.msi|*.pkg)
        artifacts+=("${artifact}")
        ;;
    esac
  done
done
if (( ${#artifacts[@]} == 0 )); then
  echo "No top-level platform release packages found under ${RELEASE_DIR}." >&2
  exit 1
fi

GNUPG_HOME="$(mktemp -d "/tmp/cc-release-sign.XXXXXX")"
chmod 700 "${GNUPG_HOME}"
mkdir -m 700 "${GNUPG_HOME}/private-keys-v1.d"
cleanup() {
  gpgconf --homedir "${GNUPG_HOME}" --kill gpg-agent >/dev/null 2>&1 || true
  rm -rf "${GNUPG_HOME}"
}
trap cleanup EXIT
for keyring_file in pubring.kbx trustdb.gpg; do
  if [[ ! -f "${SOURCE_GNUPG_HOME}/${keyring_file}" ]]; then
    echo "Required keyring file is missing: ${SOURCE_GNUPG_HOME}/${keyring_file}" >&2
    exit 1
  fi
  cp "${SOURCE_GNUPG_HOME}/${keyring_file}" "${GNUPG_HOME}/"
done
secret_keys=("${SOURCE_GNUPG_HOME}"/private-keys-v1.d/*.key)
if [[ ! -f "${secret_keys[0]}" ]]; then
  echo "No private key material found in ${SOURCE_GNUPG_HOME}/private-keys-v1.d" >&2
  exit 1
fi
cp "${secret_keys[@]}" "${GNUPG_HOME}/private-keys-v1.d/"
if [[ -d "${SOURCE_GNUPG_HOME}/openpgp-revocs.d" ]]; then
  cp -R "${SOURCE_GNUPG_HOME}/openpgp-revocs.d" "${GNUPG_HOME}/"
fi
export GNUPGHOME="${GNUPG_HOME}"
RELEASE_KEY_ID="${RELEASE_KEY_ID:-$("${GPG_BIN}" --with-colons --list-secret-keys |
  awk -F: '$1 == "ssb" && $12 ~ /s/ { print $5; exit }')}"
if [[ -z "${RELEASE_KEY_ID}" ]]; then
  echo "No signing (S) subkey found in ${SOURCE_GNUPG_HOME}." >&2
  exit 1
fi

if [[ ! -s "${PUBKEY_SOURCE}" ]]; then
  mkdir -p "$(dirname "${PUBKEY_SOURCE}")"
  "${GPG_BIN}" --armor --export "${RELEASE_KEY_ID}" > "${PUBKEY_SOURCE}"
fi
cp "${PUBKEY_SOURCE}" "${PUBLIC_KEY}"

for artifact in "${artifacts[@]}"; do
  signature="${artifact}.asc"
  rm -f "${signature}"
  "${GPG_BIN}" --batch --yes --local-user "${RELEASE_KEY_ID}" \
    --detach-sign --armor --output "${signature}" "${artifact}"
  "${GPG_BIN}" --verify "${signature}" "${artifact}"
done

python3 - "${RELEASE_DIR}" <<'PY'
from hashlib import sha256
from pathlib import Path
import sys

root = Path(sys.argv[1])
package_suffixes = (".zip", ".tar.gz", ".deb", ".rpm", ".dmg", ".exe", ".msi", ".pkg")
files = [root / "ReleaseSigningPublicKey.asc"]
for platform in sorted(path for path in root.iterdir() if path.is_dir()):
    for artifact in sorted(platform.iterdir()):
        if artifact.is_file() and artifact.name.endswith(package_suffixes):
            files.extend((artifact, artifact.with_name(artifact.name + ".asc")))

lines = []
for path in sorted(files, key=lambda item: item.relative_to(root).as_posix()):
    if not path.is_file():
        raise SystemExit(f"Expected release file is missing: {path}")
    digest_state = sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest_state.update(block)
    digest = digest_state.hexdigest()
    lines.append(f"{digest}  {path.relative_to(root).as_posix()}\n")
(root / "SHA256SUMS").write_text("".join(lines), encoding="utf-8")
PY

rm -f "${MANIFEST_SIGNATURE}"
"${GPG_BIN}" --batch --yes --local-user "${RELEASE_KEY_ID}" \
  --detach-sign --armor --output "${MANIFEST_SIGNATURE}" "${MANIFEST}"
"${GPG_BIN}" --verify "${MANIFEST_SIGNATURE}" "${MANIFEST}"

python3 - "${RELEASE_DIR}" <<'PY'
from hashlib import sha256
from pathlib import Path
import sys

root = Path(sys.argv[1])
for line in (root / "SHA256SUMS").read_text(encoding="utf-8").splitlines():
    expected, relative = line.split("  ", 1)
    digest_state = sha256()
    with (root / relative).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest_state.update(block)
    actual = digest_state.hexdigest()
    if actual != expected:
        raise SystemExit(f"Checksum mismatch: {relative}")
print(f"Verified checksums for {(root / 'SHA256SUMS').read_text().count(chr(10))} release files.")
PY

echo "Signed and verified release ${VERSION}."
echo "Signing subkey: ${RELEASE_KEY_ID}"
echo "Manifest:       ${MANIFEST}"
echo "Signature:      ${MANIFEST_SIGNATURE}"
