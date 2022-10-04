#include "cart.h"

void cart_init(struct cart* cart)
{
	switch (cart->type) {
	case 0: break;
	case 1:
		cart->prg_rom_bank_mode = 3;
		cart->chr_rom_bank_mode = 1;
		cart->mmc1_bt = 0;
		cart->mmc1_sr = 0;
		break;
	}
}

u8 cart_read(struct cart* cart, u16 addr)
{
	switch (cart->type) {
	case 0: return nrom_read(cart, addr);
	case 1: return mmc1_read(cart, addr);
	}

	return 0;
}

void cart_write(struct cart* cart, u16 addr, u8 val)
{
	switch (cart->type) {
	case 0:	nrom_write(cart, addr, val); break;
	case 1: mmc1_write(cart, addr, val); break;
	}
}

u8 cart_chr_read(struct cart* cart, u16 addr)
{
	switch (cart->type) {
	case 0: return nrom_chr_read(cart, addr);
	case 1: return mmc1_chr_read(cart, addr);
	}

	return 0;
}

void cart_chr_write(struct cart* cart, u16 addr, u8 val)
{
	switch (cart->type) {
	case 0: nrom_chr_write(cart, addr, val); break;
	case 1: mmc1_chr_write(cart, addr, val); break;
	}
}

u8 nrom_chr_read(struct cart* cart, u16 addr)
{
	return cart->chr[addr % cart->chr_sz];
}

void nrom_chr_write(struct cart* cart, u16 addr, u8 val)
{
	if(cart->chr_ram)
		cart->chr[addr % cart->chr_sz] = val;
}

u8 nrom_read(struct cart* cart, u16 addr)
{
	R(0x6000, 0x7fff, cart->prg_ram[addr - 0x6000]);
	R(0x8000, 0xffff, cart->prg[(addr - 0x8000) % cart->prg_sz]);

	return 0;
}

void nrom_write(struct cart* cart, u16 addr, u8 val)
{
	W(0x6000, 0x7fff, cart->prg_ram[addr - 0x6000] = val);
}

u8 mmc1_read(struct cart* cart, u16 addr)
{
	u16 prg_bank0 = 0, prg_bank1 = 0;

	switch(cart->prg_rom_bank_mode) {
	case 0: case 1: // 32kb mode
		prg_bank0 = cart->prg_bank & ~1;
		R(0x8000, 0xffff, cart->prg[addr - 0x8000 + prg_bank0 * 0x4000]);
		break;
	case 2: // first bank at $8000 and switch bank at $c000
		prg_bank0 = 0;
		prg_bank1 = cart->prg_bank;
		break;
	case 3: // switch bank at $8000 and last bank at $8000
		prg_bank0 = cart->prg_bank;
		prg_bank1 = cart->prg_banks - 1;
		break;
	}

	R(0x6000, 0x7fff && cart->prg_ram_enable, cart->prg_ram[addr - 0x6000]);
	R(0x8000, 0xbfff, cart->prg[(addr - 0x8000 + 0x4000 * prg_bank0)]);
	R(0xc000, 0xffff, cart->prg[(addr - 0xc000 + 0x4000 * prg_bank1)]);

	return 0;
}


void mmc1_reg_write(struct cart* cart, u16 addr, u8 val)
{
	if (val & 0x80) {
		cart->mmc1_bt = 0;
		cart->mmc1_sr = 0;
		cart->prg_rom_bank_mode = 3;
	}
	else {
		cart->mmc1_sr >>= 1;
		cart->mmc1_sr |= (val & 1) << 4;

		if (cart->mmc1_bt++ == 4) {
			switch (addr >> 12) {
			case 0x8: case 0x9: // ctrl
				switch (cart->mmc1_sr & 3) {
				case 0: case 1: nes.ppu.mirror = ONESCRM; break;
				case 2: nes.ppu.mirror = VERTICM; break;
				case 3: nes.ppu.mirror = HORIZOM; break;
				}

				cart->prg_rom_bank_mode = (cart->mmc1_sr & 0xc) >> 2;
				cart->chr_rom_bank_mode = (cart->mmc1_sr & 0x10) >> 4;
				break;
			case 0xa: case 0xb: // chr bank 0
				cart->chr_bank0 = cart->mmc1_sr;
				break;
			case 0xc: case 0xd: // chr bank 1
				cart->chr_bank1 = cart->mmc1_sr;
				break;
			case 0xe: case 0xf: // prg bank
				cart->prg_bank = (cart->mmc1_sr & 0xf);
				cart->prg_ram_enable = !(cart->mmc1_sr & 0x10);
				break;
			}

			cart->mmc1_sr = 0;
			cart->mmc1_bt = 0;
		}
	}
}

void mmc1_write(struct cart* cart, u16 addr, u8 val)
{
	W(0x6000, 0x7fff, if(cart->prg_ram_enable) cart->prg_ram[addr - 0x6000] = val);
	W(0x8000, 0xffff, mmc1_reg_write(cart, addr, val));
}

u8 mmc1_chr_read(struct cart* cart, u16 addr)
{
	if (!cart->chr_rom_bank_mode) {
		R(0x0000, 0x1fff, cart->chr[(addr + 0x1000 * (cart->chr_bank0 & ~1)) % cart->chr_sz])
	}
	else {
		R(0x0000, 0x0fff, cart->chr[(addr + 0x1000 * cart->chr_bank0) % cart->chr_sz]);
		R(0x1000, 0x1fff, cart->chr[(addr - 0x1000 + 0x1000 * cart->chr_bank1) % cart->chr_sz]);
	}

	return 0;
}

void mmc1_chr_write(struct cart* cart, u16 addr, u8 val)
{
	if (!cart->chr_ram) return;
	
	if (!cart->chr_rom_bank_mode) {
		W(0x0000, 0x1fff, cart->chr[(addr + 0x1000 * (cart->chr_bank0 & ~1)) % cart->chr_sz] = val);
	}
	else {
		W(0x0000, 0x0fff, cart->chr[(addr + 0x1000 * cart->chr_bank0) % cart->chr_sz] = val);
		W(0x1000, 0x1fff, cart->chr[(addr - 0x1000 + 0x1000 * cart->chr_bank1) % cart->chr_sz] = val);
	}
}