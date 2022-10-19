#include "nes.h"

int nes_init()
{
	nes.cpu.cart = &nes.cart;
	ppu_init(&nes.ppu);
	if (window_init(&nes.window) < 0) {
		puts("error initializing window");
		return 0;
	}

	return 1;
}

u8 nes_dbg_io_read(u16 addr)
{
	switch (addr) {
	case 0x4016: 
		return (nes.joyp & 1) | (nes.ppu.open_bus & 0xfe);
	case 0x4017: break;
	}

	return 0;
}

u8 nes_io_read(u16 addr)
{
	u8 s;

	switch (addr) {
	case 0x4016: 
		if (nes.strobe) return (nes.joyp & 1) | (nes.ppu.open_bus & 0xfe);
		else {
			if (nes.jidx > 8) nes.jidx = 0;
			return ((nes.joyp >> nes.jidx++) & 1) | (nes.ppu.open_bus & 0xf8);
		}
		
		break;
	case 0x4017: break;
	}

	return 0;
}

void nes_io_write(u16 addr, u8 val)
{
	switch (addr) {
	case 0x4016: 
		nes.strobe = val & 1;
		if (nes.strobe) nes.jidx = 0;
		break;
	case 0x4017: break;
	}
}

u8 nes_dbg_read(u16 addr)
{
	R(0x0000, 0x1fff, nes.cpu.ram[addr % 0x800]);
	R(0x2000, 0x3fff, ppu_dbg_io_read(&nes.ppu, addr));
	R(0x4000, 0x4017, nes_dbg_io_read(addr));
	R(0x4018, 0xffff, cart_read(&nes.cart, addr));

	return 0;
}

u8 _nes_read(u16 addr)
{
	R(0x0000, 0x1fff, nes.cpu.ram[addr % 0x800]);
	R(0x2000, 0x3fff, ppu_io_read(&nes.ppu, addr));
	R(0x4000, 0x4017, nes_io_read(addr));
	R(0x4014, 0x4014, ppu_io_read(&nes.ppu, addr));
	R(0x4018, 0xffff, cart_read(&nes.cart, addr));

	return 0;
}

void _nes_write(u16 addr, u8 val)
{
	W(0x0000, 0x1fff, nes.cpu.ram[addr % 0x800] = val);
	W(0x2000, 0x3fff, ppu_io_write(&nes.ppu, addr, val));
	W(0x4000, 0x4017, nes_io_write(addr, val));
	W(0x4014, 0x4014, ppu_io_write(&nes.ppu, addr, val));
	W(0x4018, 0xffff, cart_write(&nes.cart, addr, val));
}

u8 nes_read(u16 addr)
{
	nes_cycle();
	return _nes_read(addr);
}

void nes_write(u16 addr, u8 val)
{
	nes_cycle();
	_nes_write(addr, val);
}

u16 nes_read16(u16 addr)
{
	return nes_read(addr) | (nes_read(addr + 1) << 8);
}

void nes_write16(u16 addr, u16 val)
{
	nes_write(addr, val & 0xff);
	nes_write(addr + 1, val >> 8);
}

void nes_cycle()
{
	nes.cpu.cycles++;
	
	ppu_tick(&nes.ppu);
	ppu_tick(&nes.ppu);
	ppu_tick(&nes.ppu);
}

int nes_mapper_supported()
{
	const u8 ines_mappers[256] = { 
		0, 1, 4
	};
	
	int i = 0;

	for (; i < 0x100; i++) {
		if (nes.cart.type == ines_mappers[i])
			return 1;
	}

	return 0;
}