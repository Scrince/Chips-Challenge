#!/usr/bin/env python3
"""Compare the native decoder with a separate Python CHIPS.DAT parser."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import subprocess
import sys
from pathlib import Path


def load_independent_parser():
    parser_file = Path(__file__).resolve().with_name("reference_levelpack.py")
    spec = importlib.util.spec_from_file_location("ccspeedrun_levelpack", parser_file)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Cannot load independent parser at {parser_file}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module.parse_levelpack


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", required=True, type=Path, help="chips-levelpack-dump executable")
    parser.add_argument("--pack", required=True, type=Path, help="canonical CHIPS.DAT")
    parser.add_argument("--report", type=Path, help="optional path for the JSON comparison report")
    args = parser.parse_args()

    pack_bytes = args.pack.read_bytes()
    source = load_independent_parser()(args.pack)
    native_output = subprocess.run(
        [str(args.native.resolve()), str(args.pack.resolve())],
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )
    native = json.loads(native_output.stdout)
    mismatches: list[dict[str, object]] = []
    if len(source) != len(native):
        mismatches.append({"record": "campaign", "field": "level_count", "independent": len(source), "native": len(native)})

    for index, (py_level, cpp_level) in enumerate(zip(source, native), start=1):
        fields = {
            "number": (py_level.number, cpp_level["number"]),
            "time_limit": (py_level.time_limit, cpp_level["time_limit"]),
            "chips_required": (py_level.chips_required, cpp_level["chips_required"]),
            "map_type": (py_level.map_type, cpp_level["map_type"]),
            "title": (py_level.fields.get("title", ""), cpp_level["title"]),
            "hint": (py_level.fields.get("hint", ""), cpp_level["hint"]),
            "password": (py_level.fields.get("password", ""), cpp_level["password"]),
            "top_layer": (py_level.top.hex(), cpp_level["top_hex"]),
            "bottom_layer": (py_level.bottom.hex(), cpp_level["bottom_hex"]),
            "optional_fields": (
                [(field_id, raw.hex()) for field_id, raw in py_level.raw_fields],
                [(field["id"], field["raw_hex"]) for field in cpp_level["fields"]],
            ),
        }
        for field_name, (expected, actual) in fields.items():
            if expected != actual:
                mismatches.append({"level": index, "field": field_name})

    report = {
        "source_pack": str(args.pack.resolve()),
        "source_sha256": hashlib.sha256(pack_bytes).hexdigest(),
        "independent_parser": "scripts/reference_levelpack.py (stdlib-only reference parser)",
        "native_parser": str(args.native.resolve()),
        "independent_level_count": len(source),
        "native_level_count": len(native),
        "compared_fields_per_level": [
            "number", "time_limit", "chips_required", "map_type", "title", "hint", "password",
            "top_layer[1024]", "bottom_layer[1024]", "all_optional_fields_including_unknown_ids",
        ],
        "exact_match": not mismatches,
        "mismatch_count": len(mismatches),
        "mismatches": mismatches,
    }
    encoded = json.dumps(report, indent=2, ensure_ascii=False) + "\n"
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(encoded, encoding="utf-8")
    print(encoded, end="")
    return 0 if report["exact_match"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
