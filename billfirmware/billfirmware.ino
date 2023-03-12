/*
   Copyright (c) 2019 MysteryH4ck
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/bill
 */

#include "config.h"
#include "eeprom.h"
#include "ir.h"
#include "led.h"
#include "button.h"
#include "player.h"
#include "unicorn.h"

const uint8_t secret[] PROGMEM = {
  0, 6, 8, 16, 18, 22, 24, 32, 38, 40, 44, 48, 52, 56, 60, 66, 68, 70, 72, 74, 76, 78,
  82, 84, 86, 88, 90, 92, 94, 98, 100, 102, 104, 106, 108, 110, 114, 116, 118, 122,
  124, 126, 130, 132, 134, 136, 138, 140, 142, 146, 148, 150, 152, 156, 160, 164
};
uint16_t note_time[56];

int secret_index = 0;
unsigned long int secret_start = 0;
int button_state = 0;
unsigned long int button_debounce_counter = 0;
unsigned long int first_note = 0;

void unicorn_loop() {
  while (1) {
    if (button::pressed()) {
      unicorn::wololo();
    } else {
      unicorn::recharge();

      if (ir::update()) {
        uint16_t msg = ir::get_msg();
        if (msg == PARTY_CODE) {
          unicorn::party();
        } else if (msg == PARTY_HARD_CODE) {
          unicorn::party_hard();
        }
      }
    }

    unicorn::update();
    led::update();
  }
}

void track_secret() {
  unsigned long int m = millis();
  if (m < button_debounce_counter) return;
  button_debounce_counter = m + 150;

  // reset if no note has been played for two seconds
  if (secret_index > 0 && m - first_note - note_time[secret_index - 1] > 2000) secret_index = 0;

  // store note time
  if (secret_index == 0) {
    note_time[secret_index] = 0;
    first_note = m;
  } else {
    note_time[secret_index] = m - first_note;
  }
  /*
  debug(secret_index, DEC);
  debug(" <= ");
  debugln(m, DEC);
  debugln("");
  */
  secret_index++;
  if (secret_index == 56) {
    secret_index = 0;
    unsigned long int ten_bars = note_time[54];
    /*
    debug(ten_bars, DEC);
    debug(" ");
    debugln();
    */
    bool correct = true;
    for (int i = 0; i < 56; i++) {
      long int d = ((long int)note_time[i]) * 160 / ten_bars;
      uint8_t s = pgm_read_byte_near(secret + i);
      /*
      debug(i, DEC);
      debug(" ");
      debug(note_time[i], DEC);
      debug(" ");
      debug(d, DEC);
      debug(" ");
      debugln(s, DEC);
      */
      if (abs(d - s) > 1) {
        correct = false;
        break;
      }
    }
    if (correct) player::cycle_team();
  }
}

void handle_press(int state) {
  /*
  debug(button_debounce_counter, DEC);
  debug(" ");
  debug(state, DEC);
  debug(" ");
  debug(button_state, DEC);
  debugln();
  */
  if (state == 1 && button_state == 0) {
    button_state = state;
    track_secret();
  } else if (state == 0 && button_state == 1) {
    button_state = state;
  }
}

void player_loop() {
    while (1) {
        if (button::pressed()) {
            player::wololo();
            handle_press(1);
        } else {
            player::recharge();

      if (ir::update()) {
        uint16_t msg = ir::get_msg();

        if (msg == PARTY_CODE) {
          unicorn::party();
        } else if (msg == PARTY_HARD_CODE) {
          unicorn::party_hard();
        } else {
          player::convert(msg);
        }
      }
      handle_press(0);
    }

    player::update();
    led::update();
  }
}

void setup() {
  debug_init();
  debugln("Booting Bill... please stand by...");

  eeprom::begin();
  ir::begin();
  led::begin();
  button::begin();
  player::begin();
  unicorn::begin();

  debugln("Bill up and running!");

  if (unicorn::enabled()) {
    unicorn_loop();
  } else {
    player_loop();
  }
}

void loop() {}
