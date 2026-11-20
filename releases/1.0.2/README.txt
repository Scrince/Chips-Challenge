Chip's Challenge Native Remake — Release 1.0.2

Available packages:
- windows/ — Windows x64 portable package
- macos/ — ARM64, x86_64, and universal app ZIPs
- linux/ — Linux x86_64 tarball, built and validated in Ubuntu 24.04 Docker
- webassembly/ — optimized browser package; extract and deploy over HTTP/HTTPS

Each platform folder contains a SHA256SUMS.txt manifest for its package
archives. Every package archive has a detached OpenPGP signature (.asc), and
the aggregate SHA256SUMS manifest is signed as well.

Import the public key and verify the aggregate manifest:

  gpg --import ReleaseSigningPublicKey.asc
  gpg --verify SHA256SUMS.asc SHA256SUMS
  sha256sum -c SHA256SUMS  # Linux
  shasum -a 256 -c SHA256SUMS  # macOS

To verify an individual package signature, run:

  gpg --verify <package-archive>.asc <package-archive>

These are OpenPGP signatures. macOS notarization and Windows Authenticode
signing are not configured for this release.