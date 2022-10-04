#pragma once

#include "def.h"

struct cart {
	u16 type;

	u32 prg_banks;
	u32 chr_banks;
	u32 prg_sz;
	u32 chr_sz;
	u32 prg_ram_sz;

	u8 chr_ram;
	u8* prg;
	u8* chr;
	u8* prg_ram;
	u8 prg_ram_enable;

	u8 prg_bank;
	u8 chr_bank0;
	u8 chr_bank1;

	u8 prg_rom_bank_mode;
	u8 chr_rom_bank_mode;

	u8 mmc1_sr;
	u8 mmc1_bt;
	u8 mmc1_ctrl;
};

void cart_init(struct cart*);
u8 cart_read(struct cart*, u16);
void cart_write(struct cart*, u16, u8);
u8 cart_chr_read(struct cart*, u16);
void cart_chr_write(struct cart*, u16, u8);


/*nightmare*/
u8 nrom_read(struct cart*, u16);
void nrom_write(struct cart*, u16, u8);
u8 nrom_chr_read(struct cart*, u16);
void nrom_chr_write(struct cart*, u16, u8);

u8 mmc1_read(struct cart*, u16);
void mmc1_write(struct cart*, u16, u8);
u8 mmc1_chr_read(struct cart*, u16);
void mmc1_chr_write(struct cart*, u16, u8);

#include "nes.h"