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

	u8 mmc3_r[8];
	u8 mmc3_reg;
	u8 mmc3_data;
	u8 mmc3_irq_state;
	u8 mmc3_irq_latch;
	u8 mmc3_irq_ctr;
	u8 mmc3_clock;
	u8 mmc3_reload;
	int a12_wait;
	u16 clocks;
};

void cart_init(struct cart*);
u8 cart_read(struct cart*, u16);
void cart_write(struct cart*, u16, u8);
u8 cart_chr_read(struct cart*, u16);
void cart_chr_write(struct cart*, u16, u8);
void cart_a12(struct cart* cart, u16);

// mapper 0
u8 nrom_read(struct cart*, u16);
void nrom_write(struct cart*, u16, u8);
u8 nrom_chr_read(struct cart*, u16);
void nrom_chr_write(struct cart*, u16, u8);

// mapper 1
u8 mmc1_read(struct cart*, u16);
void mmc1_write(struct cart*, u16, u8);
u8 mmc1_chr_read(struct cart*, u16);
void mmc1_chr_write(struct cart*, u16, u8);

// mapper 2
u8 uxrom_read(struct cart*, u16);
void uxrom_write(struct cart*, u16, u8);
u8 uxrom_chr_read(struct cart*, u16);
void uxrom_chr_write(struct cart*, u16, u8);

// mapper 3
u8 cnrom_read(struct cart*, u16);
void cnrom_write(struct cart*, u16, u8);
u8 cnrom_chr_read(struct cart*, u16);
void cnrom_chr_write(struct cart*, u16, u8);

// mapper 4
u8 mmc3_read(struct cart*, u16);
void mmc3_write(struct cart*, u16, u8);
u8 mmc3_chr_read(struct cart*, u16);
void mmc3_chr_write(struct cart*, u16, u8);
void mmc3_a12(struct cart* cart, u16);

// mapper 7
u8 axrom_read(struct cart*, u16);
void axrom_write(struct cart*, u16, u8);
u8 axrom_chr_read(struct cart*, u16);
void axrom_chr_write(struct cart*, u16, u8);

#include "nes.h"