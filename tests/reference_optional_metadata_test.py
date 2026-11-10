from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile

reference_path = Path(__file__).resolve().parents[1] / "scripts" / "reference_levelpack.py"
spec = importlib.util.spec_from_file_location("chips_reference_optional", reference_path)
if spec is None or spec.loader is None:
    raise RuntimeError("Could not load the independent level-pack parser")
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)

def word(value: int) -> bytes:
    return value.to_bytes(2, "little")

def sized(value: bytes) -> bytes:
    return word(len(value)) + value

layer = bytes((0xFF, 0, 0)) * 4
fields = bytes((1, 2, 42, 0, 2, 2, 3, 0, 6, 4, 0xD8, 0xDB, 0xDA, 0,
                0xFE, 1, 0xAB))
record = (word(1) + word(90) + word(7) + word(1) + sized(layer) +
          sized(layer) + sized(fields))
pack = word(0xAAAC) + word(2) + word(1) + sized(record)
with tempfile.TemporaryDirectory(prefix="chips-reference-metadata-") as directory:
    pack_path = Path(directory) / "synthetic.dat"
    pack_path.write_bytes(pack)
    levels = module.parse_levelpack(pack_path)
    cli_result = subprocess.run(
        [sys.executable, str(reference_path), str(pack_path), "--level", "1"],
        check=False,
        capture_output=True,
        text=True,
    )
    if cli_result.returncode != 0:
        raise SystemExit(f"Reference parser CLI failed: {cli_result.stderr.strip()}")
    cli_level = json.loads(cli_result.stdout)

level = levels[0]
if level.time_limit != 42 or level.chips_required != 3:
    raise SystemExit("Optional fields 1 and 2 did not override header metadata")
if level.fields.get("password") != "ABC":
    raise SystemExit("Encoded optional password field was not decoded")
if level.raw_fields != ((1, bytes((42, 0))), (2, bytes((3, 0))),
                        (6, bytes((0xD8, 0xDB, 0xDA, 0))), (0xFE, bytes((0xAB,)))):
    raise SystemExit("Optional override fields were not preserved byte-for-byte")
try:
    serialized = json.dumps(level.to_dict())
except TypeError as error:
    raise SystemExit(f"Decoded level JSON contains a non-serializable value: {error}")
roundtrip = json.loads(serialized)
if roundtrip["raw_fields"] != [[1, [42, 0]], [2, [3, 0]],
                               [6, [0xD8, 0xDB, 0xDA, 0]], [0xFE, [0xAB]]]:
    raise SystemExit("JSON inspection did not retain unknown and known raw field bytes")
if cli_level["raw_fields"] != roundtrip["raw_fields"] or cli_level["top"] != list(level.top):
    raise SystemExit("Reference parser CLI changed the decoded map or raw optional metadata")
print("Independent parser optional metadata override check passed.")
