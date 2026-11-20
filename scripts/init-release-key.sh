#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GPG_BIN="${GPG_BIN:-gpg}"
GNUPG_HOME="${GNUPGHOME:-${ROOT_DIR}/.gnupg-release}"
PUBKEY_PATH="${ROOT_DIR}/docs/ChipsChallenge_Release_Signing_2026_pubkey.asc"
USER_ID="Chips Challenge Release Signing (2026) <release@chipschallenge.local>"
TEMP_GNUPG_HOME="$(mktemp -d "/tmp/cc-release-key.XXXXXX")"
BATCH_FILE="$(mktemp "/tmp/cc-release-key-batch.XXXXXX")"
chmod 700 "${TEMP_GNUPG_HOME}"
cleanup() {
  gpgconf --homedir "${TEMP_GNUPG_HOME}" --kill gpg-agent >/dev/null 2>&1 || true
  rm -rf "${TEMP_GNUPG_HOME}"
  rm -f "${BATCH_FILE}"
}
trap cleanup EXIT

if ! command -v "${GPG_BIN}" >/dev/null 2>&1; then
  echo "GnuPG was not found. Install GnuPG before creating the release key." >&2
  exit 1
fi

if [[ -e "${GNUPG_HOME}/private-keys-v1.d" ]] &&
   find "${GNUPG_HOME}/private-keys-v1.d" -type f -name '*.key' -print -quit | grep -q .; then
  echo "A secret key already exists in ${GNUPG_HOME}; refusing to overwrite it." >&2
  exit 1
fi

mkdir -p "${GNUPG_HOME}"
chmod 700 "${GNUPG_HOME}"
export GNUPGHOME="${TEMP_GNUPG_HOME}"

cat > "${BATCH_FILE}" <<EOF
%no-protection
Key-Type: RSA
Key-Length: 4096
Key-Usage: cert
Name-Real: Chips Challenge Release Signing (2026)
Name-Email: release@chipschallenge.local
Expire-Date: 2y
%commit
EOF
"${GPG_BIN}" --batch --generate-key "${BATCH_FILE}"
rm -f "${BATCH_FILE}"
PRIMARY_FINGERPRINT="$("${GPG_BIN}" --with-colons --list-keys "${USER_ID}" |
  awk -F: '$1 == "fpr" { print $10; exit }')"
if [[ -z "${PRIMARY_FINGERPRINT}" ]]; then
  echo "Could not read the new release key fingerprint." >&2
  exit 1
fi

# Keep each subkey's purpose explicit. GnuPG's pinentry protects the secret
# material; the private keyring remains local and is excluded from source control.
"${GPG_BIN}" --batch --pinentry-mode loopback --passphrase "" \
  --quick-add-key "${PRIMARY_FINGERPRINT}" ed25519 sign 2y
"${GPG_BIN}" --batch --pinentry-mode loopback --passphrase "" \
  --quick-add-key "${PRIMARY_FINGERPRINT}" cv25519 encr 2y
"${GPG_BIN}" --batch --pinentry-mode loopback --passphrase "" \
  --quick-add-key "${PRIMARY_FINGERPRINT}" ed25519 auth 2y

mkdir -p "$(dirname "${PUBKEY_PATH}")"
"${GPG_BIN}" --armor --export "${PRIMARY_FINGERPRINT}" > "${PUBKEY_PATH}"
chmod 644 "${PUBKEY_PATH}"

# Keep only regular keyring files in the repository directory. GnuPG sockets
# need a local filesystem, so agent state stays in TEMP_GNUPG_HOME.
cp "${TEMP_GNUPG_HOME}/pubring.kbx" "${GNUPG_HOME}/"
cp "${TEMP_GNUPG_HOME}/trustdb.gpg" "${GNUPG_HOME}/"
mkdir -p "${GNUPG_HOME}/private-keys-v1.d" "${GNUPG_HOME}/openpgp-revocs.d"
cp "${TEMP_GNUPG_HOME}"/private-keys-v1.d/*.key "${GNUPG_HOME}/private-keys-v1.d/"
cp "${TEMP_GNUPG_HOME}"/openpgp-revocs.d/*.rev "${GNUPG_HOME}/openpgp-revocs.d/"
chmod -R go-rwx "${GNUPG_HOME}"

echo
echo "Chips Challenge release key created."
echo "Private keyring: ${GNUPG_HOME} (GnuPG agent state uses local temporary storage)"
echo "Public key:      ${PUBKEY_PATH}"
echo "Primary key:     ${PRIMARY_FINGERPRINT}"
echo
"${GPG_BIN}" --homedir "${TEMP_GNUPG_HOME}" --fingerprint "${PRIMARY_FINGERPRINT}"
