from __future__ import annotations

import importlib.util
from pathlib import Path
import sys

reference_path = Path(__file__).resolve().parents[1] / "scripts" / "reference_levelpack.py"
spec = importlib.util.spec_from_file_location("chips_reference_levelpack", reference_path)
if spec is None or spec.loader is None:
    raise RuntimeError("Could not load the independent level-pack parser")
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)

encoded = bytes((0xFF, 0, 0x7C)) + bytes((0xFF, 0xFF, 0x42)) * 3 + bytes((0xFF, 3, 0x42))
expanded = module._expand_rle(encoded, "synthetic zero-count run")
if expanded != bytes((0x7C,)) * 256 + bytes((0x42,)) * 768:
    raise SystemExit("Independent parser did not expand RLE count zero to 256 tiles")
print("Independent parser zero-count RLE check passed.")
