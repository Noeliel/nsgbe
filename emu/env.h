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


#ifndef GameGirlColor_env_h
#define GameGirlColor_env_h

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/*--------------------MISC--------------------*/

//#define __DEBUG 1
#ifdef __DEBUG
#define DEBUG_PRINT(x) (debugprint x)
#else
#define DEBUG_PRINT(x) ( NULL )
#endif

extern _Bool do_print_debug; // set this to 1 to print debugging info (if __DEBUG is defined at compile time)

__fortify_function int debugprint(const char *__restrict __fmt, ...)
{
    if (do_print_debug)
        printf(__fmt, __va_arg_pack());
    
    return 0;
}

typedef uint8_t byte;

#ifndef __LITTLE_ENDIAN__
// try to use endian.h
// #if __BYTE_ORDER == __LITTLE_ENDIAN
// #define __LITTLE_ENDIAN__ 1
// #endif
#define GGC_LITTLE_ENDIAN   0x41424344UL
#define GGC_BIG_ENDIAN      0x44434241UL
#define GGC_PDP_ENDIAN      0x42414443UL
#define GGC_ENDIAN_ORDER    ('ABCD')
#if GGC_ENDIAN_ORDER == GGC_LITTLE_ENDIAN
#define __LITTLE_ENDIAN__ 1
#endif
#endif

typedef union {
    uint16_t w;

    struct {
#ifdef __LITTLE_ENDIAN__
        byte l, h;
#else
        byte h, l;
#endif
    } b;
} word;

#define word(value) ((word)(uint16_t)(value))

struct ROM_HEADER {
    byte start_vector[4];
    byte ninty_logo[48];
    byte game_title[15];
    byte gbc_flag;
    byte new_licensee_code[2];
    byte sgb_flag;
    byte cartridge_type;
    byte rom_size;
    byte ram_size;
    byte destination_code; // 0x00 = japan, 0x01 = anywhere else
    byte old_licensee_code;
    byte rom_version;
    byte header_checksum;
    byte global_checksum[2];
};

extern struct ROM_HEADER *rom_header;
extern void *rombuffer;
extern uintptr_t romsize;

extern void *biosbuffer;
extern uintptr_t biossize;

extern _Bool system_overclock;

extern void system_run();
extern void battery_save(byte **battery_banks, uint16_t bank_count);
extern void battery_load(byte **battery_banks, uint16_t bank_count);
extern void write_battery();

/*---------------------CPU-----------------------*/

#define FLAG_CARRY    (1 << 4)
#define FLAG_HCARRY   (1 << 5)
#define FLAG_SUBTRACT (1 << 6)
#define FLAG_ZERO     (1 << 7)

union CPU_FLAG { // apparently bitfield order is a thing, usually tied to endianess (compiler handles it though)
    struct __attribute__((packed)) {
#ifdef __LITTLE_ENDIAN__
        byte unused : 4;
        _Bool C : 1; // carry
        _Bool H : 1; // half carry
        _Bool N : 1; // subtract
        _Bool Z : 1; // zero
#else
        _Bool Z : 1;
        _Bool N : 1;
        _Bool H : 1;
        _Bool C : 1;
        byte unused : 4;
#endif
    };
    byte b;
};

struct CPU_REGS {
    union {
        struct {
#ifdef __LITTLE_ENDIAN__
            union CPU_FLAG F;
            byte A;
#else
            byte A;
            union CPU_FLAG F;
#endif
        };
        uint16_t AF;
    };

    union {
        struct {
#ifdef __LITTLE_ENDIAN__
            byte C;
            byte B;
#else
            byte B;
            byte C;
#endif
        };
        uint16_t BC;
    };

    union {
        struct {
#ifdef __LITTLE_ENDIAN__
            byte E;
            byte D;
#else
            byte D;
            byte E;
#endif
        };
        uint16_t DE;
    };

    union {
        struct {
#ifdef __LITTLE_ENDIAN__
            byte L;
            byte H;
#else
            byte H;
            byte L;
#endif
        };
        uint16_t HL;
    };

    uint16_t PC;
    uint16_t SP;
};

extern struct CPU_INSTRUCTION;

extern struct CPU_REGS cpu_regs;
extern _Bool cpu_alive;
extern byte interrupt_master_enable;

extern void cpu_reset();
extern uint32_t cpu_step();
extern int32_t cpu_exec_cycles(uint32_t clock_cycles_to_execute);
extern void cpu_break();
extern struct CPU_INSTRUCTION *cpu_next_instruction();
extern void handle_interrupts();

/*---------------------MEMORY--------------------*/

union INTERRUPT_REG {
    struct __attribute__((packed)) {
#ifdef __LITTLE_ENDIAN__
        _Bool VBLANK   : 1;
        _Bool LCD_STAT : 1;
        _Bool TIMER    : 1;
        _Bool SERIAL   : 1;
        _Bool JOYPAD   : 1;
        byte unused    : 3;
#else
        byte unused    : 3;
        _Bool JOYPAD   : 1;
        _Bool SERIAL   : 1;
        _Bool TIMER    : 1;
        _Bool LCD_STAT : 1;
        _Bool VBLANK   : 1;
#endif
    };
    byte b;
};

union MEMORY {
    struct {
        /* perm   */ byte rom_bank[0x4000];                    // 16kB..0x0000 - 0x3FFF // cartridge mapped
        /* switch */ byte rom_bank_s[0x4000];                  // 16kB..0x4000 - 0x7FFF // switchable for roms > 32kB
        /* perm   */ byte video_ram[0x2000];                   //  8kB..0x8000 - 0x9FFF // VRAM
        /* switch */ byte cart_ram_bank_s[0x2000];             //  8kB..0xA000 - 0xBFFF // external ram (cartridge/built-in nvram, eg. used for savegames)
        /* perm   */ byte ram_bank_0[0x1000];                  //  4kB..0xC000 - 0xCFFF // internal ram (W(orking)RAM)
        /* perm   */ byte ram_bank_1[0x1000];                  //  4kB..0xD000 - 0xDFFF // internal ram (W(orking)RAM)
        /* perm   */ byte undefined[0x1E00];                   //  7kB..0xE000 - 0xFDFF // internal ram, mirror of 0xC000 - 0xDDFF (ram_bank_0 & ram_bank_1)
        /* perm   */ byte sprite_attr_table[0xA0];             // 160B..0xFE00 - 0xFE9F // internal ram
        /* perm   */ byte prohibited[0x60];                    //  96B..0xFEA0 - 0xFEFF // not usable
        /* perm   */ byte dev_maps1[0xF];                      //  14B..0xFF00 - 0xFF0E // fixed device maps 1
        /* perm   */ union INTERRUPT_REG interrupt_flag_reg;   //   1B..0xFF0F
        /* perm   */ byte dev_maps2[0x70];                     // 113B..0xFF10 - 0xFF7F // fixed device maps 2
        /* perm   */ byte high_ram[0x7F];                      // 127B..0xFF80 - 0xFFFE // special WRAM
        /* perm   */ union INTERRUPT_REG interrupt_enable_reg; //   1B..0xFFFF
    } map;
    byte raw[0x10000]; // 64kB
};

extern union MEMORY mem; // due to endianess & mapping you shouldn't access this directly; instead, use the 4 functions below

extern byte mem_read(uint16_t offset);
extern word mem_read_16(uint16_t offset);
extern void mem_write(uint16_t offset, byte data);
extern void mem_write_16(uint16_t offset, word data);

extern _Bool enable_bootrom;

/*-----------------------IO-----------------------*/

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
    byte b;
};

// gameboy joypad byte encoding
union JOYPAD_IO {
    struct __attribute__((packed)) {
#ifdef __LITTLE_ENDIAN__
        _Bool RA    : 1; // right (direction) or A (action)
        _Bool LB    : 1; // left (direction) or B (action)
        _Bool USEL  : 1; // up (direction) or SELECT (action)
        _Bool DSTR  : 1; // down (direction) or START (action)
        _Bool DIR   : 1; // interpret as direction
        _Bool ACT   : 1; // interpret as action
        byte unused : 2; // unused
#else
        byte unused : 2; // unused
        _Bool ACT   : 1; // interpret as action
        _Bool DIR   : 1; // interpret as direction
        _Bool DSTR  : 1; // down (direction) or START (action)
        _Bool USEL  : 1; // up (direction) or SELECT (action)
        _Bool LB    : 1; // left (direction) or B (action)
        _Bool RA    : 1; // right (direction) or A (action)
#endif
    };
    byte b;
};

extern int32_t io_exec_cycles(uint32_t clock_cycles_to_execute);
extern uint16_t io_interpret_read(uint16_t offset);
extern uint16_t io_interpret_write(uint16_t offset, byte data);

/*--------------------EXT_CHIP--------------------*/

extern uint16_t rom_bank_count;
extern uint16_t ext_ram_bank_count;
extern byte **rom_banks;
extern byte **ext_ram_banks;
extern word active_rom_bank;
extern word active_ext_ram_bank;

extern _Bool battery_enabled;

extern uint16_t (* active_mbc_writes_interpreter)(uint16_t offset, byte data);
extern uint16_t (* active_mbc_reads_interpreter)(uint16_t offset);

extern uint32_t mbc3_setup();
extern uint32_t mbc5_setup();

/*------------------DISPLAY/PPU--------------------*/

#define OAM     0xFE00 // (r/w) sprite attribute table
#define OAM_END 0xFE9F

#define LCDC 0xFF40 // (r/w) lcd control
#define STAT 0xFF41 // (r/w) lcd status

#define SCY  0xFF42 // (r/w) scroll y
#define SCX  0xFF43 // (r/w) scroll x

#define LY   0xFF44 // (ro) lcdc y-coord
#define LYC  0xFF45 // (r/w) ly compare

#define DMA  0xFF46 // (r/w) dma transfer and start address

#define BGP  0xFF47 // (r/w) bg palette data (not on gameboy color)
#define OBP0 0xFF48 // (r/w) object palette 0 data (not on gameboy color)
#define OBP1 0xFF49 // (r/w) object palette 1 data (not on gameboy color)

#define WY   0xFF4A // (r/w) window y position
#define WX   0xFF4B // (r/w) window x position + 7

#define BCPS 0xFF68 // (?) background color palette specification (only on gameboy color)
#define BGPI 0xFF68 // (?) background palette index (same as above) (only on gameboy color)
#define BCPD 0xFF69 // (?) background color palette data (only on gameboy color)
#define BGPD 0xFF69 // (?) background palette data (same as above) (only on gameboy color)
#define OCPS 0xFF6A // (?) object color palette specification (only on gameboy color)
#define OBPI 0xFF6A // (?) sprite palette index (same as above) (only on gameboy color)
#define OCPD 0xFF6B // (?) object color palette data (only on gameboy color)
#define OBPD 0xFF6B // (?) sprite palette data (same as above) (only on gameboy color)

#define PPU_HBLANK_MODE    0 // -> 1
#define PPU_VBLANK_MODE    1 // -> 2
#define PPU_OAM_READ_MODE  2 // -> 3
#define PPU_VRAM_READ_MODE 3 // -> 0

struct PPU_REGS {
    union PPU_LCDC {
        struct __attribute__((packed)) {
#ifdef __LITTLE_ENDIAN__
            _Bool bg_window_enable_prio    : 1; // 0 = off, 1 = on
            _Bool obj_enable               : 1; // 0 = off, 1 = on
            _Bool obj_size                 : 1; // 0 = 8x8, 1 = 8x16
            _Bool bg_tile_map_area         : 1; // 0 = 9800-9BFF, 1 = 9C00-9FFF
            _Bool bg_window_tile_data_area : 1; // 0 = 8800-97FF, 1 = 8000-8FFF
            _Bool window_enable            : 1; // 0 = off, 1 = on
            _Bool window_tile_map_area     : 1; // 0 = 9800-9BFF, 1 = 9C00-9FFF
            _Bool lcd_ppu_enable           : 1; // 0 = off, 1 = on
#else
            _Bool lcd_ppu_enable           : 1;
            _Bool window_tile_map_area     : 1;
            _Bool window_enable            : 1;
            _Bool bg_window_tile_data_area : 1;
            _Bool bg_tile_map_area         : 1;
            _Bool obj_size                 : 1;
            _Bool obj_enable               : 1;
            _Bool bg_window_enable_prio    : 1;
#endif
        };
        byte b;
    } *lcdc;

    union PPU_STAT {
        struct __attribute__((packed)) {
#ifdef __LITTLE_ENDIAN__
            byte mode           : 2; // 0 = PPU_HBLANK_MODE, 1 = PPU_VBLANK_MODE, 2 = PPU_OAM_READ_MODE, 3 = PPU_VRAM_READ_MODE
            _Bool lyc_eq_ly     : 1; // 0 = different, 1 = equal
            _Bool hblank_int    : 1; // 1 = enable
            _Bool vblank_int    : 1; // 1 = enable
            _Bool oam_int       : 1; // 1 = enable
            _Bool lyc_eq_ly_int : 1; // 1 = enable
            _Bool unused        : 1;
#else
            _Bool unused        : 1;
            _Bool lyc_eq_ly_int : 1;
            _Bool oam_int       : 1;
            _Bool vblank_int    : 1;
            _Bool hblank_int    : 1;
            _Bool lyc_eq_ly     : 1;
            byte mode           : 2;
#endif
        };
        byte b;
    } *stat;
};

extern struct PPU_REGS ppu_regs;
extern _Bool ppu_alive;

extern void ppu_reset();
extern void ppu_step();
extern int32_t ppu_exec_cycles(uint32_t clock_cycles_to_execute);
extern void ppu_break();

extern uint16_t ppu_interpret_read(uint16_t offset);
extern uint16_t ppu_interpret_write(uint16_t offset, byte data);

/*---------------------NOTES----------------------*/

/*
 CPU Flag register (F) bits
 +-+-+-+-+-+-+-+-+
 |7|6|5|4|3|2|1|0|
 +-+-+-+-+-+-+-+-+
 |Z|N|H|C|0|0|0|0|
 +-+-+-+-+-+-+-+-+
 Z = Zero
 N = Subtract
 H = Half Carry
 C = Carry
 0 = Not used (always 0)
*/

#endif
