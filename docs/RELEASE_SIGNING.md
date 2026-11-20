# Release signing

Chips Challenge releases use an isolated GnuPG keyring in `.gnupg-release/`.
The keyring is private and must stay out of source control and release archives.
Each application should use its own release key; do not copy another project's
private keyring into this repository.

The key follows the same role layout used by the sibling application releases:

- A 4096-bit RSA certify-only **C** primary key identifies and manages the release key.
- An Ed25519 **S** subkey signs packages and the checksum manifest.
- A Curve25519 **E** subkey is reserved for encryption.
- An Ed25519 **A** subkey is reserved for authentication.

Only the S subkey is used to sign release artifacts. Signatures provide
integrity and authenticity when checked against the trusted public-key
fingerprint. They do not replace macOS Developer ID signing/notarization or
Windows Authenticode signing.

## Initialize once

Run this on the maintainer's signing machine. To match the sibling project
keyrings, the generated private keys have no passphrase. Their protection relies
on the `.gnupg-release/` directory's owner-only permissions; keep that directory
private and make an encrypted, offline backup of the keyring and its revocation
certificate:

```sh
scripts/init-release-key.sh
```

The script exports the public key to
`docs/ChipsChallenge_Release_Signing_2026_pubkey.asc`. Record and publish the
full primary fingerprint through a separate trusted channel. Never publish or
copy the contents of `.gnupg-release/`.

## Sign a completed release

Place each platform's final distributable package directly in its platform
folder under `releases/<version>/`, for example:

```text
releases/1.0.2/
  linux/Chips-Challenge-1.0.2-Linux-x86_64.tar.gz
  macos/Chips-Challenge-1.0.2-macOS-arm64.zip
  macos/Chips-Challenge-1.0.2-macOS-x86_64.zip
  macos/Chips-Challenge-1.0.2-macOS-universal.zip
  windows/Chips-Challenge-1.0.2-Windows-x64.zip
  webassembly/Chips-Challenge-1.0.2-WebAssembly.zip
```

After all final package bytes are in place, run:

```sh
scripts/sign-release.sh 1.0.2
```

This creates an ASCII-armored detached `.asc` signature beside every package,
copies the public key into the version folder, writes `SHA256SUMS`, and signs
that manifest as `SHA256SUMS.asc`. It verifies every signature and checksum
before reporting success. Re-run it after any package changes.

## Verify a release

First compare the primary fingerprint in the public key with the fingerprint
published through the trusted channel. Then, from the release folder:

```sh
gpg --import ReleaseSigningPublicKey.asc
gpg --verify SHA256SUMS.asc SHA256SUMS
shasum -a 256 -c SHA256SUMS
```

For an individual package, verify its detached signature with:

```sh
gpg --verify linux/Chips-Challenge-1.0.2-Linux-x86_64.tar.gz.asc \
  linux/Chips-Challenge-1.0.2-Linux-x86_64.tar.gz
```
