#include "cart.h"

void cart_init(struct cart* cart)
{
	switch (cart->type) {
	case 1:
		cart->prg_rom_bank_mode = 3;
		cart->chr_rom_bank_mode = 1;
		cart->mmc1_bt = 0;
		cart->mmc1_sr = 0;
		cart->prg_ram_enable = 1;
		break;
	case 4:
		cart->prg_banks *= 2;
		cart->chr_banks /= 2;
		cart->prg_ram_enable = 1;
		cart->a12_wait = 10;
		break;
	}
}

// OPTIMIZATION: FUNCTION TABLES!!!!!!

u8 cart_read(struct cart* cart, u16 addr)
{
	switch (cart->type) {
	case 0: return nrom_read(cart, addr);
	case 1: return mmc1_read(cart, addr);
	case 2: return uxrom_read(cart, addr);
	case 3: return cnrom_read(cart, addr);
	case 4: return mmc3_read(cart, addr);
	case 7: return axrom_read(cart, addr);
	}

	return 0;
}

void cart_write(struct cart* cart, u16 addr, u8 val)
{
	switch (cart->type) {
	case 0:	nrom_write(cart, addr, val); break;
	case 1: mmc1_write(cart, addr, val); break;
	case 2: uxrom_write(cart, addr, val); break;
	case 3: cnrom_write(cart, addr, val); break;
	case 4: mmc3_write(cart, addr, val); break;
	case 7: axrom_write(cart, addr, val); break;
	}
}

u8 cart_chr_read(struct cart* cart, u16 addr)
{
	switch (cart->type) {
	case 0: return nrom_chr_read(cart, addr);
	case 1: return mmc1_chr_read(cart, addr);
	case 2: return uxrom_chr_read(cart, addr);
	case 3: return cnrom_chr_read(cart, addr);
	case 4: return mmc3_chr_read(cart, addr);
	case 7: return axrom_chr_read(cart, addr);
	}

	return 0;
}

void cart_chr_write(struct cart* cart, u16 addr, u8 val)
{
	switch (cart->type) {
	case 0: nrom_chr_write(cart, addr, val); break;
	case 1: mmc1_chr_write(cart, addr, val); break;
	case 2: uxrom_chr_write(cart, addr, val); break;
	case 3: cnrom_chr_write(cart, addr, val); break;
	case 4: mmc3_chr_write(cart, addr, val); break;
	case 7: axrom_chr_write(cart, addr, val); break;
	}
}

void cart_a12(struct cart* cart, u16 addr)
{
	switch (cart->type) {
	case 4: mmc3_a12(cart, addr); break;
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
				case 0: case 1: nes.ppu.mirror = ONESC1M; break;
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

u8 uxrom_read(struct cart* cart, u16 addr)
{
	R(0x8000, 0xbfff, cart->prg[addr - 0x8000 + cart->prg_bank * 0x4000]);
	R(0xc000, 0xffff, cart->prg[addr - 0xc000 + (cart->prg_banks-1) * 0x4000]);

	return 0;
}

void uxrom_write(struct cart* cart, u16 addr, u8 val)
{
	W(0x8000, 0xffff, cart->prg_bank = val & 0xf);
}

u8 uxrom_chr_read(struct cart* cart, u16 addr)
{
	R(0x0000, 0x1fff, cart->chr[addr]);

	return 0;
}

void uxrom_chr_write(struct cart* cart, u16 addr, u8 val)
{
	if (cart->chr_ram)
		cart->chr[addr % cart->chr_sz] = val;
}

u8 cnrom_read(struct cart* cart, u16 addr)
{
	R(0x8000, 0xffff, cart->prg[(addr - 0x8000) % cart->prg_sz]);

	return 0;
}

void cnrom_write(struct cart* cart, u16 addr, u8 val)
{
	W(0x8000, 0xffff, cart->chr_bank0 = val);
}

u8 cnrom_chr_read(struct cart* cart, u16 addr)
{
	R(0x0000, 0x1fff, cart->chr[(addr + 0x2000 * cart->chr_bank0) % cart->chr_sz]);

	return 0;
}

void cnrom_chr_write(struct cart* cart, u16 addr, u8 val)
{
	if (cart->chr_ram)
		cart->chr[(addr + 0x1000 * cart->chr_bank0) % cart->chr_sz] = val;
}

u8 mmc3_read(struct cart* cart, u16 addr)
{
	R(0x6000, 0x7fff, cart->prg_ram_enable ? cart->prg_ram[addr - 0x6000] : nes.ppu.open_bus);
	R(0x8000, 0x9fff, cart->prg[addr - 0x8000 + 0x2000 * 
		(cart->prg_rom_bank_mode ? cart->prg_banks - 2 : cart->mmc3_r[6])]);
	R(0xa000, 0xbfff, cart->prg[addr - 0xa000 + 0x2000 * cart->mmc3_r[7]]);
	R(0xc000, 0xdfff, cart->prg[addr - 0xc000 + 0x2000 *
		(cart->prg_rom_bank_mode ? cart->mmc3_r[6] : cart->prg_banks - 2)]);
	R(0xe000, 0xffff, cart->prg[addr - 0xe000 + 0x2000 * (cart->prg_banks - 1)]);

	return 0;
}

void mmc3_bank_select(struct cart* cart, u8 val)
{
	cart->mmc3_reg = val & 7;
	cart->prg_rom_bank_mode = !!(val & 0x40);
	cart->chr_rom_bank_mode = !!(val & 0x80);
}

void mmc3_a12(struct cart* cart, u16 addr)
{
	u16 a12 = !!(addr & 0x1000), irq=0;

	if (!cart->mmc3_clock && a12) {
		if (cart->mmc3_irq_ctr == 0 || cart->mmc3_reload) {
			cart->mmc3_irq_ctr = cart->mmc3_irq_latch;
		}
		else {
			--cart->mmc3_irq_ctr;
		}

		if (cart->mmc3_irq_ctr == 0 && cart->mmc3_irq_state) {
			nes.cpu.irq = 1;
		}

		cart->mmc3_reload = 0;
	}

	cart->mmc3_clock = a12;
}

void mmc3_write(struct cart* cart, u16 addr, u8 val)
{
	W(0x6000, 0x7fff, if (cart->prg_ram_enable) cart->prg_ram[addr - 0x6000] = val);
	if (!(addr & 1)) { // even
		W(0x8000, 0x9ffe, mmc3_bank_select(cart, val)); // bank select
		W(0xa000, 0xbffe, nes.ppu.mirror = !(val & 1)); // mirroring
		W(0xc000, 0xdffe, cart->mmc3_irq_latch = val); // irq latch
		W(0xe000, 0xfffe, cart->mmc3_irq_state = 0; nes.cpu.irq = 0); // irq disable
	}
	else { // odd
		W(0x8001, 0x9fff, cart->mmc3_r[cart->mmc3_reg] = val); // bank data
		W(0xa001, 0xbfff, cart->prg_ram_enable = !!(val & 0xc0)); // PRG RAM protect
		W(0xc001, 0xdfff, cart->mmc3_irq_ctr = 0; cart->mmc3_reload = 1); // irq reload
		W(0xe001, 0xffff, cart->mmc3_irq_state = 1); // irq enable
	}
}

u8 mmc3_chr_read(struct cart* cart, u16 addr)
{
	// shorten this pl0x...
	if (cart->chr_rom_bank_mode) {
		R(0x0000, 0x03ff, cart->chr[addr - 0x0000 + 0x400 * cart->mmc3_r[2]]);
		R(0x0400, 0x07ff, cart->chr[addr - 0x0400 + 0x400 * cart->mmc3_r[3]]);
		R(0x0800, 0x0bff, cart->chr[addr - 0x0800 + 0x400 * cart->mmc3_r[4]]);
		R(0x0c00, 0x0fff, cart->chr[addr - 0x0c00 + 0x400 * cart->mmc3_r[5]]);
		R(0x1000, 0x17ff, cart->chr[addr - 0x1000 + 0x400 * cart->mmc3_r[0]]);
		R(0x1800, 0x1fff, cart->chr[addr - 0x1800 + 0x400 * cart->mmc3_r[1]]);
	}
	else {
		R(0x0000, 0x07ff, cart->chr[addr - 0x0000 + 0x400 * cart->mmc3_r[0]]);
		R(0x0800, 0x0fff, cart->chr[addr - 0x0800 + 0x400 * cart->mmc3_r[1]]);
		R(0x1000, 0x13ff, cart->chr[addr - 0x1000 + 0x400 * cart->mmc3_r[2]]);
		R(0x1400, 0x17ff, cart->chr[addr - 0x1400 + 0x400 * cart->mmc3_r[3]]);
		R(0x1800, 0x1bff, cart->chr[addr - 0x1800 + 0x400 * cart->mmc3_r[4]]);
		R(0x1c00, 0x1fff, cart->chr[addr - 0x1c00 + 0x400 * cart->mmc3_r[5]]);
	}

	return 0;
}

void mmc3_chr_write(struct cart* cart, u16 addr, u8 val)
{
	if (cart->chr_rom_bank_mode) {
		W(0x0000, 0x03ff, cart->chr[addr - 0x0000 + 0x400 * cart->mmc3_r[2]] = val);
		W(0x0400, 0x07ff, cart->chr[addr - 0x0400 + 0x400 * cart->mmc3_r[3]] = val);
		W(0x0800, 0x0bff, cart->chr[addr - 0x0800 + 0x400 * cart->mmc3_r[4]] = val);
		W(0x0c00, 0x0fff, cart->chr[addr - 0x0c00 + 0x400 * cart->mmc3_r[5]] = val);
		W(0x1000, 0x17ff, cart->chr[addr - 0x1000 + 0x400 * cart->mmc3_r[0]] = val);
		W(0x1800, 0x1fff, cart->chr[addr - 0x1800 + 0x400 * cart->mmc3_r[1]] = val);
	}
	else {
		W(0x0000, 0x07ff, cart->chr[addr - 0x0000 + 0x400 * cart->mmc3_r[0]] = val);
		W(0x0800, 0x0fff, cart->chr[addr - 0x0800 + 0x400 * cart->mmc3_r[1]] = val);
		W(0x1000, 0x13ff, cart->chr[addr - 0x1000 + 0x400 * cart->mmc3_r[2]] = val);
		W(0x1400, 0x17ff, cart->chr[addr - 0x1400 + 0x400 * cart->mmc3_r[3]] = val);
		W(0x1800, 0x1bff, cart->chr[addr - 0x1800 + 0x400 * cart->mmc3_r[4]] = val);
		W(0x1c00, 0x1fff, cart->chr[addr - 0x1c00 + 0x400 * cart->mmc3_r[5]] = val);
	}
}

u8 axrom_read(struct cart* cart, u16 addr)
{
	R(0x8000, 0xffff, cart->prg[addr - 0x8000 + 0x8000 * cart->prg_bank]);

	return 0;
}

void axrom_write(struct cart* cart, u16 addr, u8 val)
{
	W(0x8000, 0xffff, cart->prg_bank = val & 7; nes.ppu.mirror = (val & 0x10) ? ONESC1M : ONESC2M; );
}

u8 axrom_chr_read(struct cart* cart, u16 addr)
{
	R(0x0000, 0x1fff, cart->chr[addr]);

	return 0;
}

void axrom_chr_write(struct cart* cart, u16 addr, u8 val)
{
	if (cart->chr_ram)
		cart->chr[addr % cart->chr_sz] = val;
}