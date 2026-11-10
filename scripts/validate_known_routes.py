#!/usr/bin/env python3
"""Replay external, previously solved campaign routes through the native engine."""

from __future__ import annotations

import argparse
import json
import subprocess
import tempfile
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runner", required=True, type=Path, help="path to chips-replay executable")
    parser.add_argument("--pack", required=True, type=Path, help="path to canonical CHIPS.DAT")
    parser.add_argument("--routes", required=True, type=Path, help="solved-route JSON from an independent solver")
    parser.add_argument("--report", required=True, type=Path, help="destination machine-readable JSON report")
    parser.add_argument("--seed", type=int, default=1, help="initial native RNG seed (default: 1)")
    parser.add_argument("--ticks-per-input", type=int, default=2,
                        help="simulation ticks between successive route inputs (default: 2)")
    parser.add_argument("--expect-wins", default="",
                        help="optional comma-separated level numbers that must reach a native win")
    args = parser.parse_args()
    if args.ticks_per_input < 1:
        parser.error("--ticks-per-input must be at least 1")
    try:
        expected_wins = {int(value) for value in args.expect_wins.split(",") if value}
    except ValueError:
        parser.error("--expect-wins must be a comma-separated list of positive level numbers")
    if any(level < 1 for level in expected_wins):
        parser.error("--expect-wins level numbers must be positive")

    route_rows = json.loads(args.routes.read_text(encoding="utf-8"))
    results: list[dict[str, object]] = []
    for row in route_rows:
        if row.get("status") != "solved" or not row.get("route"):
            continue
        level = int(row["level"])
        route = str(row["route"]).upper()
        input_mode = str(row.get("input_mode", "queued"))
        if input_mode not in ("queued", "press", "settled_press"):
            raise ValueError(f"Level {level}: unsupported input mode {input_mode!r}")
        operation = "press" if input_mode in ("press", "settled_press") else "move"
        commands = [f"level {level}", f"seed {args.seed}"]
        route_by_step: dict[int, int] = {}
        replay_step = 0
        for route_index, direction in enumerate(route):
            if direction not in "NESW":
                raise ValueError(f"Level {level}: unsupported route direction {direction!r}")
            commands.append(f"{operation} {direction}")
            replay_step += 1
            route_by_step[replay_step] = route_index
            if input_mode == "settled_press":
                commands.append("settle 128")
                replay_step += 1
                route_by_step[replay_step] = route_index
            else:
                commands.append(f"advance {args.ticks_per_input}")
                replay_step += 1
                route_by_step[replay_step] = route_index

        with tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=".replay", delete=False) as replay_file:
            replay_file.write("\n".join(commands) + "\n")
            replay_path = Path(replay_file.name)
        try:
            completed = subprocess.run(
                [str(args.runner), str(args.pack), str(replay_path)],
                check=False, capture_output=True, text=True,
            )
        finally:
            replay_path.unlink(missing_ok=True)

        if completed.returncode != 0:
            results.append({"level": level, "route_moves": len(route), "status": "runner_error",
                            "error": completed.stderr.strip() or completed.stdout.strip()})
            continue
        snapshots = [json.loads(line) for line in completed.stdout.splitlines() if line.strip()]
        if not snapshots:
            results.append({"level": level, "route_moves": len(route), "status": "empty_output"})
            continue
        final = snapshots[-1]
        terminal = next((snapshot for snapshot in snapshots if snapshot["outcome"] != "playing"), None)
        route_index = route_by_step.get(int(terminal["step"])) if terminal is not None else None
        results.append({
            "level": level,
            "route_moves": len(route),
            "status": "won" if final["outcome"] == "won" else "not_won",
            "native_outcome": final["outcome"],
            "native_tick": final["tick"],
            "native_player_position": final["player"],
            "native_chips_remaining": final["chips"],
            "first_terminal_move_index": route_index,
            "first_terminal_move": route[route_index] if route_index is not None else None,
            "first_terminal_tick": terminal["tick"] if terminal is not None else None,
            "first_terminal_position": terminal["player"] if terminal is not None else None,
            "source_route_status": row.get("status"),
            "source_completed_by_player": row.get("completed_by_player"),
            "input_mode": input_mode,
        })

    report = {
        "schema_version": 1,
        "engine": "chips-replay",
        "pack": str(args.pack),
        "routes": str(args.routes),
        "seed": args.seed,
        "ticks_per_input": args.ticks_per_input,
        "expected_wins": sorted(expected_wins),
        "route_count": len(results),
        "won_count": sum(result["status"] == "won" for result in results),
        "results": results,
    }
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if expected_wins:
        results_by_level = {int(result["level"]): result for result in results}
        missing_levels = sorted(expected_wins - results_by_level.keys())
        failed_levels = sorted(level for level in expected_wins
                               if level in results_by_level and results_by_level[level]["status"] != "won")
        required_wins = len(expected_wins) - len(missing_levels)
        print(f"Required native route wins: {required_wins}/{len(expected_wins)} completed")
        if missing_levels:
            print(f"Missing expected route levels: {missing_levels}")
        if failed_levels:
            print(f"Expected route levels that did not win: {failed_levels}")
    else:
        print(f"Native route results: {report['won_count']}/{report['route_count']} completed")
    print(f"Report: {args.report}")
    for result in results:
        print(f"Level {result['level']:>3}: {result['status']} (native outcome: {result.get('native_outcome', 'error')})")
    if expected_wins:
        return 0 if not missing_levels and not failed_levels else 1
    return 0 if report["won_count"] == report["route_count"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
