#pragma once

#include "def.h"

struct cart {
	u16 type;

	u32 prg_sz;
	u32 chr_sz;
	u8 chr_ram;
	u8* prg;
	u8* chr;
	u8* prg_ram;
};

u8 cart_read(struct cart*, u16);
void cart_write(struct cart*, u16, u8);
u8 cart_chr_read(struct cart*, u16);
void cart_chr_write(struct cart*, u16, u8);


/*nightmare*/
u8 nrom_read(struct cart*, u16);
void nrom_write(struct cart*, u16, u8);
u8 nrom_chr_read(struct cart*, u16);
void nrom_chr_write(struct cart*, u16, u8);