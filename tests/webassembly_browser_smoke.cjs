#!/usr/bin/env node
"use strict";

const assert = require("node:assert/strict");
const { chromium, firefox, webkit } = require("playwright");

const url = process.env.CHIPS_WEB_URL || "http://host.docker.internal:8765/chips-native.html";
const levelPack = process.env.CHIPS_LEVEL_PACK || "/testdata/CCLP1.dat";
const expectedPackTitle = process.env.CHIPS_EXPECT_PACK_TITLE || "Key Pyramid";
const viewport = { width: 1280, height: 820 };

async function runBrowser(name, browserType) {
  const browser = await browserType.launch({ headless: process.env.CHIPS_HEADFUL !== "1" });
  const context = await browser.newContext({ viewport, deviceScaleFactor: 1, hasTouch: true });
  const page = await context.newPage();
  const tap = async (x, y) => {
    if (process.env.CHIPS_INPUT === "mouse") await page.mouse.click(x, y);
    else await page.touchscreen.tap(x, y);
  };
  const pageErrors = [];
  const failedRequests = [];
  const badResponses = [];
  const pickerProbes = [];
  page.on("pageerror", (error) => {
    // SDL_RunApp hands control back after its asynchronous IDBFS mount with
    // Emscripten's documented "unwind" sentinel; it is not an app exception.
    if (error.message !== "unwind") pageErrors.push(error.message);
  });
  page.on("console", (message) => {
    if (message.text().startsWith("[picker-probe]")) pickerProbes.push(message.text());
  });
  page.on("requestfailed", (request) => failedRequests.push(`${request.url()}: ${request.failure()?.errorText}`));
  page.on("response", (response) => {
    if (response.status() >= 400) badResponses.push(`${response.status()} ${response.url()}`);
  });

  try {
    await page.addInitScript(() => {
      const originalClick = HTMLInputElement.prototype.click;
      HTMLInputElement.prototype.click = function () {
        if (this.type === "file")
          console.log("[picker-probe] input.click active=" + navigator.userActivation.isActive);
        return originalClick.call(this);
      };
    });
    const response = await page.goto(url, { waitUntil: "domcontentloaded", timeout: 30000 });
    assert.equal(response && response.status(), 200, `${name}: game page should load`);
    await page.waitForFunction(() => document.title.includes("Chip's Challenge:"), null, { timeout: 45000 });
    const persistentStore = await page.evaluate(async () => {
      const existingDatabases = indexedDB.databases
        ? (await indexedDB.databases()).map(({ name, version }) => ({ name, version }))
        : null;
      return new Promise((resolve, reject) => {
      let created = false;
      const request = indexedDB.open("/libsdl");
      request.onupgradeneeded = () => { created = true; };
      request.onerror = () => reject(request.error);
      request.onsuccess = () => {
        const database = request.result;
        const stores = Array.from(database.objectStoreNames);
        database.close();
        resolve({ created, stores, existingDatabases });
      };
      });
    });
    assert.equal(persistentStore.created, false, `${name}: SDL should mount its existing IndexedDB store: ${JSON.stringify(persistentStore)}`);
    assert(persistentStore.stores.includes("FILE_DATA"), `${name}: SDL IndexedDB file store is missing: ${JSON.stringify(persistentStore)}`);
    await page.waitForFunction(() => {
      const canvas = document.querySelector("canvas#canvas");
      return canvas && canvas.width > 0 && canvas.height > 0 && typeof window.chipsUnlockMusicAudio === "function";
    }, null, { timeout: 15000 });

    // First tap unlocks browser audio. Keyboard activation starts the default campaign.
    const canvas = page.locator("canvas#canvas");
    await canvas.focus();
    await tap(640, 410);
    await page.keyboard.press("Enter");
    await page.waitForTimeout(150);
    await page.keyboard.press("Enter"); // dismiss the level placard
    await page.waitForTimeout(150);
    await page.keyboard.press("M"); // change a profile setting and force a save
    await page.waitForFunction(async () => {
      const database = await new Promise((resolve, reject) => {
        const request = indexedDB.open("/libsdl");
        request.onerror = () => reject(request.error);
        request.onsuccess = () => resolve(request.result);
      });
      const keys = await new Promise((resolve, reject) => {
        const request = database.transaction("FILE_DATA", "readonly")
          .objectStore("FILE_DATA").getAllKeys();
        request.onerror = () => reject(request.error);
        request.onsuccess = () => resolve(request.result.map(String));
      });
      database.close();
      return keys.some((key) => key.includes("campaign.ccprofile"));
    }, null, { timeout: 15000 });

    // Open the Level menu and its fifth item through touch so SDL's touch-to-mouse
    // compatibility path and the trusted-gesture file picker are both exercised.
    const chooserPromise = page.waitForEvent("filechooser", { timeout: 15000 });
    if (process.env.CHIPS_INPUT === "keyboard") {
      await page.keyboard.press("F10");
      await page.keyboard.press("ArrowRight");
      await page.keyboard.press("ArrowRight");
      for (let index = 0; index < 5; index++) await page.keyboard.press("ArrowDown");
      await page.keyboard.press("Enter");
    } else {
      await tap(190, 14);
      await page.waitForTimeout(100);
      await tap(250, 132);
    }
    const chooser = await chooserPromise;
    await chooser.setFiles(levelPack);
    await page.waitForFunction(
      (expectedTitle) => document.title.includes(expectedTitle),
      expectedPackTitle,
      { timeout: 30000 },
    );

    assert.deepEqual(pageErrors, [], `${name}: uncaught browser exceptions`);
    assert.deepEqual(failedRequests, [], `${name}: failed asset requests`);
    assert.deepEqual(badResponses, [], `${name}: HTTP error responses`);
    console.log(`${name}: started WebAssembly game and loaded CCLP1 through touch file picker`);
  } catch (error) {
    if (process.env.SMOKE_ARTIFACT_DIR) {
      const fs = require("node:fs");
      fs.mkdirSync(process.env.SMOKE_ARTIFACT_DIR, { recursive: true });
      await page.screenshot({ path: `${process.env.SMOKE_ARTIFACT_DIR}/${name.toLowerCase()}.png` }).catch(() => {});
    }
    console.error(`${name}: failed at title=${await page.title().catch(() => "unavailable")}`);
    for (const probe of pickerProbes) console.error(probe);
    const inputTrace = await page.evaluate(() => {
      const fs = window.Module && window.Module.FS ? window.Module.FS : window.FS;
      if (!fs) return "FS unavailable";
      try { return new TextDecoder().decode(fs.readFile("/tmp/chips-native-input-trace.log")); }
      catch { return "input trace unavailable"; }
    }).catch(() => "page evaluation unavailable");
    console.error(`${name}: SDL input trace: ${inputTrace}`);
    throw error;
  } finally {
    await context.close();
    await browser.close();
  }
}

(async () => {
  const browsers = [["Chromium", chromium], ["Firefox", firefox], ["WebKit", webkit]];
  const selected = new Set((process.env.CHIPS_BROWSERS || "Chromium,Firefox,WebKit").split(","));
  for (const [name, browserType] of browsers.filter(([name]) => selected.has(name)))
    await runBrowser(name, browserType);
  console.log("Browser smoke matrix passed.");
})().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
