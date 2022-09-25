#include "cart.h"

u8 cart_read(struct cart* cart, u16 addr)
{
	switch (cart->type) {
	case 0:
		return nrom_read(cart, addr);
	}

	return 0;
}

void cart_write(struct cart* cart, u16 addr, u8 val)
{
	switch (cart->type) {
	case 0:
		nrom_write(cart, addr, val);
	}
}

u8 cart_chr_read(struct cart* cart, u16 addr)
{
	switch (cart->type) {
	case 0: return nrom_chr_read(cart, addr);
	}

	return 0;
}

void cart_chr_write(struct cart* cart, u16 addr, u8 val)
{
	switch (cart->type) {
	case 0: nrom_chr_write(cart, addr, val); break;
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