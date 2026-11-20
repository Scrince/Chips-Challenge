import { Sequencer, WorkletSynthesizer } from "spessasynth_lib";

const SOUND_BANK_URL = "./soundfonts/GeneralUser-GS.sf2";
const SYNTH_WORKLET_URL = new URL("./vendor/spessasynth_lib/spessasynth_processor.min.js", import.meta.url);

let context = null;
let synth = null;
let sequencer = null;
let synthPromise = null;
let volume = 0.2;
let muted = false;
let playbackId = 0;
let contextListenersAttached = false;

function ensureAudioContext(suppliedContext) {
  if (suppliedContext) {
    if (context && context !== suppliedContext)
      throw new Error("The browser MIDI player cannot switch audio contexts");
    context = suppliedContext;
  }
  if (!context) {
    const AudioContext = window.AudioContext || window.webkitAudioContext;
    if (!AudioContext) throw new Error("Web Audio is unavailable");
    context = new AudioContext();
  }
  if (!contextListenersAttached) {
    context.addEventListener("statechange", resumeWhenVisible);
    contextListenersAttached = true;
  }
  if (context.state === "suspended" && !document.hidden)
    context.resume().catch((error) => console.warn("Could not resume music audio:", error));
  return context;
}

async function loadSynth() {
  if (!synthPromise) {
    synthPromise = (async () => {
      const audioContext = ensureAudioContext();
      await audioContext.audioWorklet.addModule(SYNTH_WORKLET_URL);
      const instance = new WorkletSynthesizer(audioContext, { convolverMode: true });
      instance.connect(audioContext.destination);
      await instance.isReady;

      const response = await fetch(SOUND_BANK_URL);
      if (!response.ok) throw new Error(`Could not load the General MIDI sound bank (${response.status})`);
      await instance.soundBankManager.addSoundBank(await response.arrayBuffer(), "main");
      instance.setSystemParameter("gain", muted ? 0 : volume * 0.65);

      synth = instance;
      sequencer = new Sequencer(instance);
      sequencer.loopCount = Infinity;
      return instance;
    })().catch((error) => {
      synthPromise = null;
      throw error;
    });
  }
  return synthPromise;
}

function resumeWhenVisible() {
  if (!context || !synth) return;
  if (document.hidden) {
    if (context.state === "running")
      context.suspend().catch((error) => console.warn("Could not suspend background music:", error));
  } else if (context.state === "suspended") {
    context.resume().catch((error) => console.warn("Could not resume background music:", error));
  }
}

export async function unlock(audioContext) {
  try {
    ensureAudioContext(audioContext);
  } catch (error) {
    console.warn("Background music is unavailable:", error);
  }
}

export async function start(audioContext, path, initialVolume, initialMuted) {
  stop();
  const requestId = playbackId;
  volume = Math.max(0, Math.min(1, initialVolume));
  muted = !!initialMuted;

  try {
    ensureAudioContext(audioContext);
    const audioSynth = await loadSynth();
    if (requestId !== playbackId) return;

    const bytes = FS.readFile(path);
    if (!bytes || bytes.length < 14) throw new Error("Invalid MIDI data");
    const binary = bytes.slice().buffer;
    audioSynth.setSystemParameter("gain", muted ? 0 : volume * 0.65);
    sequencer.loadNewSongList([{ binary, fileName: path }]);
    sequencer.play();
  } catch (error) {
    stop();
    throw error;
  }
}

export function stop() {
  ++playbackId;
  if (sequencer) sequencer.pause();
  if (synth) synth.stopAll(true);
}

export function setGain(newVolume, newMuted) {
  volume = Math.max(0, Math.min(1, newVolume));
  muted = !!newMuted;
  if (synth) synth.setSystemParameter("gain", muted ? 0 : volume * 0.65);
}

document.addEventListener("visibilitychange", resumeWhenVisible);
window.addEventListener("pageshow", resumeWhenVisible);
