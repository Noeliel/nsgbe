/**
    N(o) S(pecial) G(ame) B(oy) E(mulator)
    Copyright (C) 2021  Noeliel

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/


#include <stdint.h>

//#define __DEBUG
#ifdef __DEBUG
extern _Bool activate_single_stepping_on_condition;
#endif

#ifndef __always_inline
#define __always_inline __inline __attribute__ ((__always_inline__))
#endif

#define GB_FRAMEBUFFER_WIDTH 160
#define GB_FRAMEBUFFER_HEIGHT 144

// this is a high level representation used for state transfer
// between the platform-dependend api (gtk+ for example) and
// emulation i/o
union BUTTON_STATE {
    struct __attribute__((packed)) {
#ifdef __LITTLE_ENDIAN__
        _Bool A      : 1; // action button
        _Bool B      : 1; // action button
        _Bool START  : 1; // action button
        _Bool SELECT : 1; // action button
        _Bool UP     : 1; // direction button
        _Bool DOWN   : 1; // direction button
        _Bool LEFT   : 1; // direction button
        _Bool RIGHT  : 1; // direction button
#else
        _Bool RIGHT  : 1;
        _Bool LEFT   : 1;
        _Bool DOWN   : 1;
        _Bool UP     : 1;
        _Bool SELECT : 1;
        _Bool START  : 1;
        _Bool B      : 1;
        _Bool A      : 1;
#endif
    };
    uint8_t b;
};

/*------------platform-specific gui------------*/

// copy of button states owned and modified by frontend
extern union BUTTON_STATE button_states;

// any frontend or gui provides implementations for these
extern long load_rom(uint8_t **buffer);
extern long load_bios(uint8_t **buffer);
extern long load_battery(uint8_t **buffer);
extern int save_battery(uint8_t *buffer, size_t size);

/*--------------------NSGBE----------------------*/

// frontend can toggle system overclock by changing this bool
extern _Bool system_overclock;

// run this at least once before launching the event loop
extern void system_reset();

// launch this in a new thread to run the core in a self-contained timed event loop
extern void system_run_event_loop();

// call either of these if you wish to implement your own event loop
extern void clock_perform_sleep_cycle_ticks(); // untimed
extern void clock_perform_sleep_cycle(); // timed

// frontend can pause / resume emulation using these two functions
extern void system_resume();
extern void system_pause();

// frontend uses these to interact with the core
extern void write_battery();
extern uint32_t *display_request_next_frame();

// frontend can set up a callback on this to be notified about new frames
extern void (* display_notify_vblank)();

/*--------------------MISC--------------------*/

struct ROM_HEADER {
    uint8_t start_vector[4];
    uint8_t ninty_logo[48];
    uint8_t game_title[15];
    uint8_t gbc_flag;
    uint8_t new_licensee_code[2];
    uint8_t sgb_flag;
    uint8_t cartridge_type;
    uint8_t rom_size;
    uint8_t ram_size;
    uint8_t destination_code; // 0x00 = japan, 0x01 = anywhere else
    uint8_t old_licensee_code;
    uint8_t rom_version;
    uint8_t header_checksum;
    uint8_t global_checksum[2];
};

// after rom has been loaded, this is available
extern struct ROM_HEADER *rom_header;