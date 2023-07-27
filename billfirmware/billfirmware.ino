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

#define MS_PER_BAR 2894
#define BEATS_PER_BAR 16
const uint8_t secret[] PROGMEM = {
  0, 6, 8, 16, 18, 22, 24, 32, 38, 40, 44, 48, 52, 56, 60, 66, 68, 70, 72, 74, 76, 78,
  82, 84, 86, 88, 90, 92, 94, 98, 100, 102, 104, 106, 108, 110, 114, 116, 118, 122,
  124, 126, 130, 132, 134, 136, 138, 140, 142, 146, 148, 150, 152, 156, 160, 164
};

int secret_index = 0;
int button_state = 0;
unsigned long int button_debounce_counter = 0;
unsigned long int first_note = 0;
unsigned long int latest_note = 0;

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
  debugln("oy!");
  unsigned long int m = millis();
  if (m < button_debounce_counter) return;
  button_debounce_counter = m + 150;

  // reset if no note has been played for two seconds
  if (secret_index > 0 && m - latest_note > 2000) secret_index = 0;
  latest_note = m;

  if (secret_index == 0) {
    first_note = m;
    secret_index = 1;
  } else {
    long int d = (m - first_note) * BEATS_PER_BAR / MS_PER_BAR;
    uint8_t s = pgm_read_byte_near(secret + secret_index);
    if (abs(d - s) <= 1) {
      secret_index++;
      debugln(secret_index);
      if (secret_index == 56) {
          player::cycle_team();
          secret_index = 0;
      }
    } else {
      secret_index = 0;
    }
  }
}

void handle_press() {
  if (button_state == 0) {
    button_state = 1;
    track_secret();
  }
}

void player_loop() {
    while (1) {
        if (button::pressed()) {
            player::wololo();
            handle_press();
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
            button_state = 0;
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