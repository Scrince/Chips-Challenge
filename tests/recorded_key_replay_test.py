from __future__ import annotations

import importlib.util
from pathlib import Path
import sys

script_path = Path(__file__).resolve().parents[1] / "scripts" / "replay_recorded_key_events.py"
spec = importlib.util.spec_from_file_location("chips_recorded_key_replay", script_path)
if spec is None or spec.loader is None:
    raise RuntimeError("Could not load recorded-key replay helper")
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)

capture = {
    "level": 2,
    "input_format": "key_edges_us_v1",
    "input_events": [
        {"t_us": 35_000, "key": "E", "action": "down"},
        {"t_us": 90_000, "key": "E", "action": "up"},
        {"t_us": 215_000, "key": "N", "action": "down"},
        {"t_us": 280_000, "key": "N", "action": "up"},
        {"t_us": 330_000, "key": "W", "action": "down"},
    ],
}

expected = "level 2\nseed 7\nadvance 4\npress E\nadvance 1\npress N\nadvance 2\npress W\n"
actual = module.build_replay(capture, seed=7, pre_record_ticks=4, phase_ms=0)
if actual != expected:
    raise SystemExit(f"Unexpected replay conversion:\n{actual}")

shifted = module.build_replay(capture, seed=7, pre_record_ticks=0, phase_ms=80)
if "advance 1\npress E" not in shifted:
    raise SystemExit("Tick phase did not shift the first event across its boundary")

bad_order = dict(capture)
bad_order["input_events"] = list(reversed(capture["input_events"]))
try:
    module.build_replay(bad_order, seed=1, pre_record_ticks=0, phase_ms=0)
except ValueError as error:
    if "timestamp order" not in str(error):
        raise
else:
    raise SystemExit("Out-of-order recorded key events were accepted")

print("Recorded key-event replay timing checks passed.")
