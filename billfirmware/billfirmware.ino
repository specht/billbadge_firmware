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

int secret[] =
{0,
 1032,
 1406,
 2848,
 3407,
 3951,
 4321,
 5739,
 6844,
 7212,
 7968,
 8635,
 9383,
 10089,
 10838,
 11939,
 12269,
 12641,
 12987,
 13347,
 13735,
 14079,
 14795,
 15153,
 15497,
 15885,
 16245,
 16604,
 16982,
 17692,
 18061,
 18414,
 18785,
 19117,
 19516,
 19851,
 20577,
 20957,
 21332,
 22032,
 22376,
 22732,
 23490,
 23818,
 24204,
 24542,
 24942,
 25301,
 25671,
 26367,
 26740,
 27075,
 27458,
 28179,
 28918,
 29662,
 30356,
 31778};

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
        long int diff = ((long int)m - secret_start) - secret[secret_index];
        debug(secret_index, DEC);
        debug(" ");
        debugln(diff, DEC);
        if (diff > -100 && diff < 100) {
            secret_index++;
            if (secret_index == 56) {
                debugln("HOORAY!");
                secret_index = -1;
                player::cycle_team();
            }
        } else {
            debugln("FAIL");
            secret_index = -1;
        }
    }
}

void handle_press(unsigned long int m, int state) {
    if (m > last_button_state_change + 10) {
        if (state == 1 && button_state == 0) {
            button_state = state;
            last_button_state_change = m;
            // debugln("button down!");
            track_secret(m);
            // debug(m, DEC);
            // debug(" ");
        } else if (state == 0 && button_state == 1) {
            button_state = state;
            last_button_state_change = m;
            // debugln("button up!");
            // track_secret(m);
            // debug(m, DEC);
            // debug(" ");
        }
    }
}

void player_loop() {
    while (1) {
        unsigned long int m = millis();
        if (button::pressed()) {
            player::wololo();
            handle_press(m, 1);
        } else {
            handle_press(m, 0);
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
