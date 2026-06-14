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
assertContains(rtcService, "loadCurrentFoxMorsePattern", "serviceRTCSecondTick");
assertContains(rtcService, "currentFoxShouldTransmit()", "serviceRTCSecondTick");
assertContains(rtcService, "post_sync_seconds_to_send_ID = g_id_interval_seconds", "serviceRTCSecondTick");
assertContains(rtcService, "post_sync_seconds_to_send_ID =  (slot * g_on_air_interval_seconds) - secondsForID", "serviceRTCSecondTick");
assertContains(rtcService, "stationIDMorseStart", "serviceRTCSecondTick");
assertContains(rtcService, "send_ID_now = FALSE", "serviceRTCSecondTick");
assertNotContains(rtcService, "makeMorse((char*)\" \"", "serviceRTCSecondTick");

const periodicService = extractFunction("servicePeriodicTaskTick");
assertContains(periodicService, "g_callsign_sent = TRUE", "servicePeriodicTaskTick");
assertContains(periodicService, "currentFoxShouldTransmit()", "servicePeriodicTaskTick");
assertContains(periodicService, "loadCurrentFoxMorsePattern();", "servicePeriodicTaskTick");
assertContains(periodicService, "if(g_reset_transmit_service_state)", "servicePeriodicTaskTick");
assertContains(periodicService, "codeInc = g_on_the_air ? 1 : 0", "servicePeriodicTaskTick");
assertContains(periodicService, "ptt_delay = 0", "servicePeriodicTaskTick");
assertContains(periodicService, "ptt_dropped = 0", "servicePeriodicTaskTick");
assertContains(periodicService, "writeKeyFast(OFF)", "servicePeriodicTaskTick");
assertContains(periodicService, "requestAMModulatorReset()", "servicePeriodicTaskTick");
assertContains(periodicService, "if(g_transmissions_disabled || !g_on_the_air)", "servicePeriodicTaskTick");
assertContains(periodicService, "writePttFast(OFF)", "servicePeriodicTaskTick");

const stationIDMorseStart = extractFunction("stationIDMorseStart");
assertContains(stationIDMorseStart, "while(*stationID == ' ')", "stationIDMorseStart");
assertContains(stationIDMorseStart, "stationID++", "stationIDMorseStart");

const isContinuousTransmissionMode = extractFunction("isContinuousTransmissionMode");
assertContains(isContinuousTransmissionMode, "g_number_of_foxes == 1", "isContinuousTransmissionMode");

const currentFoxShouldTransmit = extractFunction("currentFoxShouldTransmit");
assertContains(currentFoxShouldTransmit, "isContinuousTransmissionMode()", "currentFoxShouldTransmit");
assertContains(currentFoxShouldTransmit, "g_number_of_foxes > 1", "currentFoxShouldTransmit");
assertContains(currentFoxShouldTransmit, "g_fox == (g_fox_counter + g_fox_id_offset)", "currentFoxShouldTransmit");

const loadCurrentFoxMorsePattern = extractFunction("loadCurrentFoxMorsePattern");
assertContains(loadCurrentFoxMorsePattern, "copyFoxMorsePattern", "loadCurrentFoxMorsePattern");
assertContains(loadCurrentFoxMorsePattern, "g_code_throttle = THROTTLE_VAL_FROM_WPM(g_pattern_codespeed)", "loadCurrentFoxMorsePattern");
assertContains(loadCurrentFoxMorsePattern, "makeMorse((char*)\"\\0\"", "loadCurrentFoxMorsePattern");
assertContains(loadCurrentFoxMorsePattern, "makeMorse((char*)g_messages_text[PATTERN_TEXT]", "loadCurrentFoxMorsePattern");

const setupForFox = extractFunction("setupForFox");
assertContains(setupForFox, "BOOL startActiveEventNow = FALSE", "setupForFox");
assertContains(setupForFox, "g_initialize_fox_transmissions = INIT_EVENT_IN_PROGRESS_WITH_STARTFINISH_TIMES", "setupForFox");
assertContains(setupForFox, "g_transmissions_disabled = FALSE", "setupForFox");
assertContains(setupForFox, "g_transmissions_disabled = TRUE", "setupForFox");
assertContains(setupForFox, "if(startActiveEventNow && !g_thermal_shutdown)", "setupForFox");
assertContains(setupForFox, "loadCurrentFoxMorsePattern();", "setupForFox");
assertContains(setupForFox, "g_on_the_air = TRUE", "setupForFox");
assertContains(setupForFox, "requestAMModulatorReset()", "setupForFox");
assertContains(setupForFox, "if(g_transmissions_disabled || !g_on_the_air)", "setupForFox");
assertContains(setupForFox, "writePttFast(OFF)", "setupForFox");
assertContains(setupForFox, "updateAudioSamplingForAMTransmit()", "setupForFox");
assertContains(setupForFox, "g_reset_transmit_service_state = TRUE", "setupForFox");

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
assertContains(timer2, "g_reset_am_modulator_state", "TIMER2_COMPA_vect");
assertContains(timer2, "index = 0", "TIMER2_COMPA_vect");
assertContains(timer2, "PORTB", "TIMER2_COMPA_vect");
assertNotContains(timer2, "g_periodic_service_ticks", "TIMER2_COMPA_vect");

const amToneFrequency = extractFunction("setAMToneFrequency");
assertContains(amToneFrequency, "TIMSK2 |= (1 << OCIE2A)", "setAMToneFrequency");
assertContains(amToneFrequency, "TIMSK2 &= ~(1 << OCIE2A)", "setAMToneFrequency");
assertNotContains(amToneFrequency, "TIMSK1 |= (1 << OCIE1A)", "setAMToneFrequency");
assertNotContains(amToneFrequency, "TIMSK1 &= ~(1 << OCIE1A)", "setAMToneFrequency");

console.log("Firmware interrupt regression checks passed.");
