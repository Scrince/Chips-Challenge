#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -ne 1 || ! -d "$1" ]]; then
  echo "Usage: $0 <staged-release-directory>" >&2
  exit 2
fi

matches="$(find "$1" -type f \( -iname 'CCLP2.DAT' -o -iname 'CCLP2-disclaimer.txt' \) -print)"
if [[ -n "${matches}" ]]; then
  echo "CCLP2 files are not permitted in release packages:" >&2
  printf '%s\n' "${matches}" >&2
  exit 1
fi

echo "Verified no CCLP2 campaign data or disclaimer in $1."
