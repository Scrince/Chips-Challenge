(() => {
  "use strict";

  // Keep these entry points synchronous for Emscripten's JavaScript bridge;
  // the SoundFont engine itself is loaded lazily as an ES module.
  let playerPromise = null;
  let audioContext = null;
  const player = () => playerPromise || (playerPromise = import("./soundfont-midi.js"));

  function ensureAudioContext() {
    if (!audioContext) {
      const AudioContext = window.AudioContext || window.webkitAudioContext;
      if (!AudioContext) throw new Error("Web Audio is unavailable");
      audioContext = new AudioContext();
    }
    return audioContext;
  }

  window.chipsUnlockMusicAudio = () => {
    try {
      const context = ensureAudioContext();
      // Resume before awaiting any imports so Edge and other browsers still
      // see this call as part of the trusted pointer or keyboard gesture.
      context.resume().catch((error) => console.warn("Could not resume music audio:", error));
      player().then((module) => module.unlock(context)).catch((error) =>
        console.warn("Could not unlock browser music audio:", error));
    } catch (error) {
      console.warn("Background music is unavailable:", error);
    }
  };

  window.chipsStartMidiMusic = (path, initialVolume, initialMuted) => {
    try {
      const context = ensureAudioContext();
      player().then((module) => module.start(context, path, initialVolume, initialMuted)).catch((error) =>
        console.error("Could not play the browser MIDI track:", error));
    } catch (error) {
      console.error("Could not play the browser MIDI track:", error);
    }
  };

  window.chipsStopMidiMusic = () => {
    if (playerPromise) playerPromise.then((module) => module.stop());
  };

  window.chipsSetMusicGain = (newVolume, newMuted) => {
    if (playerPromise) playerPromise.then((module) => module.setGain(newVolume, newMuted));
  };
})();
