#pragma once

#include "def.h"

#define NMTA (0x2000 | (ppu->v & 0xfff))
#define ATRA (0x23c0 | (ppu->v & 0xc00) | ((ppu->v >> 4) & 0x38) | ((ppu->v >> 2) & 0x07))
#define SIZE (256 * 240)
#define PX(x,y) ppu->pixels[((x)+((y) * 256)) % SIZE]
#define REND ((ppu->mask & M_BG) || (ppu->mask & M_SPR))
#define LCLIP (!(ppu->mask & M_BG_LM) || !(ppu->mask & M_SPR_LM))
#define PAL (((addr - 0x3f00) & ~((((addr-0x3f00) & 0x13) == 0x10) ? 0x10 : 0)) & 0x1f)

/* PPUCTRL flags */
#define C_BASE_NT 3
#define C_VRAM_INC (1<<2)
#define C_SPR_PT (1<<3)
#define C_BG_PT (1<<4)
#define C_SPR_SZ (1<<5)
#define C_PPU_MS (1<<6)
#define C_NMI (1<<7)

/* PPUMASK flags */
#define M_GREY (1<<0)
#define M_BG_LM (1<<1)
#define M_SPR_LM (1<<2)
#define M_BG (1<<3)
#define M_SPR (1<<4)
#define M_RED (1<<5)
#define M_GREEN (1<<6)
#define M_BLUE (1<<7)

/* PPUSTATUS flags */
#define S_SPR_OVR (1<<5)
#define S_SPR0HIT (1<<6)
#define S_VBL (1<<7)

/* OAM attr flags */
#define A_PAL 3
#define A_PRIO (1 << 5)
#define A_FLIP_HORZ (1 << 6)
#define A_FLIP_VERT (1 << 7)

struct oam_entry {
	u8 y_coord;
	u8 tile;
	u8 attr;
	u8 x_coord;
};

struct sprite {
	u8 pthi;
	u8 ptlo;
	u8 attr;
	u8 x;
	u8 y;
};

struct ppu {
	u8 ctrl;
	u8 mask;
	u8 status;
	u8 oam_addr;
	u8 oam_data;
	u8 scroll;
	u16 ppu_addr;
	u8 ppu_data;
	u8 oam_dma_addr;
	u8 mirror;

	u16 v;
	u16 t;
	u8 x;
	u8 w;

	// bg latches
	u8 ntl;
	u8 atl;
	u16 pths;
	u16 ptls;
	u8 pth;
	u8 ptl;
	u16 aths;
	u16 atls;

	// spr eval regs
	u8 ctr;
	u8 copy;
	u8 s;
	u8 n;
	u8 m;
	u8 in_range;
	u8 done;

	// spr fetch regs
	u8 cur_spr;
	u8 sprites_found;

	// sprite other regs
	u8 spr;

	u8 vbl_block;
	u8 ppu_delay;
	u8 open_bus;
	int open_bus_decay;
	u8 ppudata_read_buffer;
	
	u16 scanline;

	u8 vram[0x800];
	struct oam_entry oam[64];
	struct oam_entry soam[8];
	struct sprite sprites[8];

	u8 pal[0x20];
	u32 pixels[SIZE];

	u16 dot;
	int frames;
};

void ppu_init(struct ppu*);
void ppu_tick(struct ppu*);
u8 ppu_dbg_io_read(struct ppu*, u16);
u8 ppu_io_read(struct ppu*, u16);
void ppu_io_write(struct ppu*, u16, u8);
u8 ppu_read(struct ppu*, u16);
void ppu_write(struct ppu*, u16, u8);

#include "nes.h"