#!/usr/bin/env node

import fs from "node:fs";
import process from "node:process";

const sourcePath = "Software/AtmelStudio7/Arducon/Arducon/main.cpp";
const source = fs.readFileSync(sourcePath, "utf8");

function fail(message) {
  throw new Error(message);
}

function extractBracedBlock(startIndex) {
  const braceStart = source.indexOf("{", startIndex);

  if (braceStart < 0) {
    fail(`Could not find opening brace after index ${startIndex}.`);
  }

  let depth = 0;
  for (let index = braceStart; index < source.length; index += 1) {
    const char = source[index];

    if (char === "{") {
      depth += 1;
    } else if (char === "}") {
      depth -= 1;
      if (depth === 0) {
        return source.slice(braceStart, index + 1);
      }
    }
  }

  fail(`Could not find matching closing brace after index ${startIndex}.`);
}

function extractFunction(name) {
  const pattern = new RegExp(`(?:^|\\n)\\s*(?:[A-Za-z_][\\w:\\s\\*&<>]*\\s+)?${name}\\s*\\([^;]*\\)\\s*\\{`, "m");
  const match = pattern.exec(source);

  if (!match) {
    fail(`Could not find function '${name}'.`);
  }

  return extractBracedBlock(match.index);
}

function extractIsr(vector) {
  const pattern = new RegExp(`ISR\\s*\\(\\s*${vector}\\s*\\)`, "m");
  const match = pattern.exec(source);

  if (!match) {
    fail(`Could not find ISR '${vector}'.`);
  }

  return extractBracedBlock(match.index);
}

function assertContains(block, needle, context) {
  if (!block.includes(needle)) {
    fail(`${context} must contain '${needle}'.`);
  }
}

function assertNotContains(block, needle, context) {
  if (block.includes(needle)) {
    fail(`${context} must not contain '${needle}'.`);
  }
}

const int0 = extractIsr("INT0_vect");
assertContains(int0, "g_rtc_service_ticks", "INT0_vect");
assertContains(int0, "g_rtc_service_ticks++", "INT0_vect");

for (const forbidden of [
  "makeMorse",
  "digitalWrite",
  "timeRequiredToSendStrAtWPM",
  "copyFoxMorsePattern",
  "sprintf",
  "strcpy",
]) {
  assertNotContains(int0, forbidden, "INT0_vect");
}

const rtcService = extractFunction("serviceRTCSecondTick");
assertContains(rtcService, "g_current_epoch++", "serviceRTCSecondTick");
assertContains(rtcService, "g_seconds_since_powerup++", "serviceRTCSecondTick");
assertContains(rtcService, "g_seconds_since_sync++", "serviceRTCSecondTick");
assertContains(rtcService, "makeMorse", "serviceRTCSecondTick");
assertContains(rtcService, "copyFoxMorsePattern", "serviceRTCSecondTick");

const loop = extractFunction("loop");
const rtcServiceIndex = loop.indexOf("servicePendingRTCSeconds();");
const periodicServiceIndex = loop.indexOf("servicePendingPeriodicTasks();");

if (rtcServiceIndex < 0) {
  fail("loop must service pending RTC seconds.");
}

if (periodicServiceIndex < 0) {
  fail("loop must service pending periodic tasks.");
}

if (rtcServiceIndex > periodicServiceIndex) {
  fail("loop must service RTC seconds before periodic tasks.");
}

const timer1 = extractIsr("TIMER1_COMPA_vect");
assertContains(timer1, "g_periodic_service_ticks", "TIMER1_COMPA_vect");
assertNotContains(timer1, "g_sendAMmodulation", "TIMER1_COMPA_vect");

const timer2 = extractIsr("TIMER2_COMPA_vect");
assertContains(timer2, "g_sendAMmodulation", "TIMER2_COMPA_vect");
assertContains(timer2, "PORTB", "TIMER2_COMPA_vect");
assertNotContains(timer2, "g_periodic_service_ticks", "TIMER2_COMPA_vect");

const amToneFrequency = extractFunction("setAMToneFrequency");
assertContains(amToneFrequency, "TIMSK2 |= (1 << OCIE2A)", "setAMToneFrequency");
assertContains(amToneFrequency, "TIMSK2 &= ~(1 << OCIE2A)", "setAMToneFrequency");
assertNotContains(amToneFrequency, "TIMSK1 |= (1 << OCIE1A)", "setAMToneFrequency");
assertNotContains(amToneFrequency, "TIMSK1 &= ~(1 << OCIE1A)", "setAMToneFrequency");

console.log("Firmware interrupt regression checks passed.");
