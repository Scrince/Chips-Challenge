#!/usr/bin/env python3
"""Replay an original key_edges_us_v1 capture through the native engine."""

from __future__ import annotations

import argparse
import json
import subprocess
import tempfile
from pathlib import Path


TICK_US = 110_000
KEYS = {"N", "E", "S", "W"}


def build_replay(recording: dict[str, object], seed: int,
                 pre_record_ticks: int, phase_ms: int) -> str:
    """Convert key-down edges to native presses on the 110 ms simulation clock."""
    if pre_record_ticks < 0:
        raise ValueError("pre-record ticks must be nonnegative")
    if not 0 <= phase_ms < 110:
        raise ValueError("phase must be in the range 0..109 ms")
    if recording.get("input_format") != "key_edges_us_v1":
        raise ValueError("recording must use input_format key_edges_us_v1")
    level = int(recording["level"])
    events = recording.get("input_events")
    if not isinstance(events, list):
        raise ValueError("recording has no input_events array")

    commands = [f"level {level}", f"seed {seed}"]
    if pre_record_ticks:
        commands.append(f"advance {pre_record_ticks}")

    phase_us = phase_ms * 1_000
    current_event_tick = 0
    for event in events:
        if not isinstance(event, dict) or event.get("action") != "down":
            continue
        key = str(event.get("key", "")).upper()
        if key not in KEYS:
            raise ValueError(f"unsupported movement key in recording: {key!r}")
        time_us = int(event["t_us"])
        if time_us < 0:
            raise ValueError("event timestamps must be nonnegative")
        event_tick = (time_us + phase_us) // TICK_US
        if event_tick < current_event_tick:
            raise ValueError("input events are not in timestamp order")
        elapsed_ticks = event_tick - current_event_tick
        if elapsed_ticks:
            commands.append(f"advance {elapsed_ticks}")
        commands.append(f"press {key}")
        current_event_tick = event_tick

    if not any(command.startswith("press ") for command in commands):
        raise ValueError("recording contains no key-down events")
    return "\n".join(commands) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("recording", type=Path, help="keyboard recorder JSON file")
    parser.add_argument("--level", type=int, help="level to select; defaults to the first capture")
    parser.add_argument("--runner", required=True, type=Path, help="path to chips-replay executable")
    parser.add_argument("--pack", required=True, type=Path, help="path to CHIPS.DAT")
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--pre-record-ticks", type=int, default=0,
                        help="simulate this many ticks before the recorder clock starts")
    parser.add_argument("--phase-ms", type=int, default=0,
                        help="offset recorder timestamps from native tick boundaries (0..109 ms)")
    parser.add_argument("--expect-outcome", choices=("playing", "won", "dead", "timed_out"),
                        help="fail if the replay does not end in this outcome")
    parser.add_argument("--expect-player", type=int,
                        help="fail unless the final row-major board position matches")
    parser.add_argument("--expect-chips", type=int,
                        help="fail unless the final remaining-chip count matches")
    parser.add_argument("--command-file", type=Path,
                        help="save the generated native replay commands for inspection")
    args = parser.parse_args()

    try:
        rows = json.loads(args.recording.read_text(encoding="utf-8"))
        candidates = [row for row in rows if args.level is None or row.get("level") == args.level]
        recording = next((row for row in candidates
                          if row.get("input_format") == "key_edges_us_v1"), None)
        if recording is None:
            raise ValueError("no matching key_edges_us_v1 recording was found")
        replay_text = build_replay(recording, args.seed,
                                   args.pre_record_ticks, args.phase_ms)
        if args.command_file:
            args.command_file.parent.mkdir(parents=True, exist_ok=True)
            args.command_file.write_text(replay_text, encoding="utf-8")

        with tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=".replay",
                                         delete=False) as replay_file:
            replay_file.write(replay_text)
            replay_path = Path(replay_file.name)
        try:
            completed = subprocess.run(
                [str(args.runner), str(args.pack), str(replay_path)],
                check=False, capture_output=True, text=True,
            )
        finally:
            replay_path.unlink(missing_ok=True)

        if completed.returncode != 0:
            print(completed.stderr.strip() or completed.stdout.strip())
            return completed.returncode
        snapshots = [json.loads(line) for line in completed.stdout.splitlines() if line.strip()]
        terminal = next((snapshot for snapshot in snapshots if snapshot["outcome"] != "playing"), None)
        final = snapshots[-1] if snapshots else {}
        print(json.dumps({
            "level": recording["level"],
            "title": recording.get("title"),
            "seed": args.seed,
            "pre_record_ticks": args.pre_record_ticks,
            "phase_ms": args.phase_ms,
            "captured_key_downs": sum(
                1 for event in recording["input_events"]
                if event.get("action") == "down"
            ),
            "outcome": final.get("outcome"),
            "tick": final.get("tick"),
            "player": final.get("player"),
            "chips_remaining": final.get("chips"),
            "first_terminal_step": terminal.get("step") if terminal else None,
            "first_terminal_tick": terminal.get("tick") if terminal else None,
        }, indent=2))
        if args.expect_outcome and final.get("outcome") != args.expect_outcome:
            print(f"Expected outcome {args.expect_outcome!r}, got {final.get('outcome')!r}.")
            return 1
        if args.expect_player is not None and final.get("player") != args.expect_player:
            print(f"Expected final player position {args.expect_player}, got {final.get('player')!r}.")
            return 1
        if args.expect_chips is not None and final.get("chips") != args.expect_chips:
            print(f"Expected {args.expect_chips} remaining chips, got {final.get('chips')!r}.")
            return 1
        return 0
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError) as error:
        parser.error(str(error))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
