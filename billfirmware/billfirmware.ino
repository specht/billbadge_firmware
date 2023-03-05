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

int secret = [0, 267, 563, 814, 986, 1262, 1438, 1565, 1712, 1817, 1970, 2089, 2264, 2469];
int secret_index = -1;
unsigned long int secret_start = 0;
int button_state = 0;
unsigned long int last_button_state_change = 0;

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

void track_secret(unsigned long int m) {
    if (secret_index == -1) {
        secret_start = m;
        secret_index = 0;
    }
    if (secret_index >= 0) {
        
    }
}

void handle_press(unsigned long int m, int state) {
    if (m > last_button_state_change + 10) {
        if (state == 1 && button_state == 0) {
            button_state = 1;
            last_button_state_change = m;
            track_secret(m);
        } else {
            button_state = 0;
            last_button_state_change = m;
            track_secret(m);
        }
    }
}

void player_loop() {
    while (1) {
        unsigned long int m = millis();
        if (button::pressed()) {
            player::wololo();
            handle_press(m, true);
        } else {
            handle_press(m, false);
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
