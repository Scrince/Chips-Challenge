#!/usr/bin/env node
"use strict";

const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const root = path.resolve(__dirname, "..");
const bridge = fs.readFileSync(path.join(root, "web/midi-player.js"), "utf8");
const player = fs.readFileSync(path.join(root, "web/soundfont-midi.js"), "utf8");
const shell = fs.readFileSync(path.join(root, "web/wasm-shell.html"), "utf8");
const soundfont = path.join(root, "web/soundfonts/GeneralUser-GS.sf2");

for (const entryPoint of [
  "chipsUnlockMusicAudio",
  "chipsStartMidiMusic",
  "chipsStopMidiMusic",
  "chipsSetMusicGain",
]) {
  assert.ok(bridge.includes(entryPoint), `${entryPoint} must remain available to the game`);
}

assert.match(player, /WorkletSynthesizer/, "music should use the SoundFont AudioWorklet engine");
assert.match(player, /new Sequencer\(/, "music should use a MIDI sequencer");
assert.match(player, /loopCount = Infinity/, "background tracks should loop continuously");
assert.match(player, /GeneralUser-GS\.sf2/, "music should load the bundled GM sound bank");
assert.match(player, /setSystemParameter\("gain"/, "music settings should control synthesizer gain");
assert.ok(fs.statSync(soundfont).size > 1_000_000, "the bundled GM sound bank must be present");

for (const dependency of ["spessasynth_lib", "spessasynth_core", "fflate", "stb-vorbis"])
  assert.ok(shell.includes(`"${dependency}"`), `${dependency} needs a local browser import-map entry`);

console.log("Browser MIDI uses the bundled SoundFont synthesizer and GM sound bank");
