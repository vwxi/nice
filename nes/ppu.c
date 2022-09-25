#include "ppu.h"

u32 ppu_clr_pal[64] = { // RGB888 colors
	0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600, 0x561D00,
	0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000, 0x000000, 0x000000,
	0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
	0x6B6D00, 0x388700, 0x0C9300, 0x008F32, 0x007C8D, 0x000000, 0x000000, 0x000000,
	0xFFFEFF, 0x64B0FF, 0x9290FF, 0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22,
	0xBCBE00, 0x88D800, 0x5CE430, 0x45E082, 0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
	0xFFFEFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5, 0xF7D8A5,
	0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000
};

u8 ppu_default_pal[32] = { // default palette RAM colors
	0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D, 
	0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
	0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14, 
	0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08 
};

void ppu_init(struct ppu* ppu)
{
	ppu->scanline = 0;
	ppu->dot = 0;
	ppu->ctrl = 0;
	ppu->mask = 0;
	ppu->status = 0;
	ppu->oam_addr = 0;
	ppu->oam_data = 0;
	ppu->scroll = 0;
	ppu->ppu_addr = 0;
	ppu->ppu_data = 0;
	ppu->oam_dma_addr = 0;
	ppu->v = ppu->t = ppu->x = ppu->w = 0;
	ppu->vbl_block = 0;
	ppu->ctr = ppu->s = 0;
	memcpy(ppu->pal, ppu_default_pal, 32);
	memset(ppu->pixels, ppu_clr_pal[0], SIZE);
}

u16 ppu_mirror(struct ppu* ppu, u16 addr)
{
	switch (ppu->mirror) {
	case 0: return ((addr >> 1) & 0x400) | (addr & 0x3ff);
	case 1: return addr & 0x7ff;
	default: return addr - 0x2000;
	}
}

u8 ppu_read(struct ppu* ppu, u16 addr)
{
	R(0x0000, 0x1fff, cart_chr_read(&nes.cart, addr));
	R(0x2000, 0x3eff, ppu->vram[ppu_mirror(ppu, addr)]);
	R(0x3f00, 0x3fff, (ppu->pal[PAL] & ~0xc0) | (ppu->open_bus & 0xc0));

	return 0;
}

void ppu_write(struct ppu* ppu, u16 addr, u8 val)
{
	W(0x0000, 0x1fff, cart_chr_write(&nes.cart, addr, val));
	W(0x2000, 0x3eff, ppu->vram[ppu_mirror(ppu, addr)] = val);
	W(0x3f00, 0x3fff, ppu->pal[PAL] = val);
}

u8 ppu_dbg_io_read(struct ppu* ppu, u16 addr)
{
	addr = 0x2000 | (addr % 8);

	switch (addr) {
	case 0x2002: return ppu->status;
	case 0x2004: return ppu->oam_data;
	case 0x2007: return ppu->ppu_data;
	}

	return 0;
}

void ppu_update_open_bus(struct ppu* ppu, u8 val)
{
	ppu->open_bus = val;
	ppu->open_bus_decay = 77777;
}

/*from nesdev.org/wiki/PPU_scrolling#Tile_and_attribute_fetching*/
void ppu_inc_x(struct ppu* ppu)
{
	if (!REND) return;

	if ((ppu->v & 0x1f) == 31) {
		ppu->v &= ~0x1f;
		ppu->v ^= 0x400;
	}
	else ppu->v++;
}

/*from nesdev.org/wiki/PPU_scrolling#Tile_and_attribute_fetching*/
void ppu_inc_y(struct ppu* ppu)
{
	int y;
	if (!REND) return;

	if ((ppu->v & 0x7000) != 0x7000) ppu->v += 0x1000;
	else {
		ppu->v &= ~0x7000;
		y = (ppu->v & 0x3e0) >> 5;
		if (y == 29) {
			y = 0;
			ppu->v ^= 0x800;
		}
		else if (y == 31) {
			y = 0;
		}
		else y++;

		ppu->v = (ppu->v & ~0x3e0) | (y << 5);
	}
}

void ppu_vert_copy(struct ppu* ppu)
{
	if (!REND) return;

	ppu->v &= ~0x7be0;
	ppu->v |= (ppu->t & 0x7be0);
}

void ppu_horz_copy(struct ppu* ppu)
{
	if (!REND) return;

	ppu->v &= ~0x41f;
	ppu->v |= (ppu->t & 0x41f);
}

void ppu_shift_update(struct ppu* ppu)
{
	ppu->ptls |= ppu->ptl;
	ppu->pths |= ppu->pth;
	ppu->aths |= (ppu->atl & 2) ? 0xff : 0;
	ppu->atls |= (ppu->atl & 1) ? 0xff : 0;
}

void ppu_shift_registers(struct ppu* ppu)
{
	ppu->ptls <<= 1;
	ppu->pths <<= 1;
	ppu->aths <<= 1;
	ppu->atls <<= 1;
}

u16 ppu_bg_addr(struct ppu* ppu, u8 tile)
{
	u16 half = !!(ppu->ctrl & C_BG_PT) << 12;
	return half | (tile << 4) | (ppu->v >> 12);
}

u16 ppu_spr_addr(struct ppu* ppu, u8 tile, u8 y)
{
	u16 half = !!(ppu->ctrl & C_SPR_PT) << 12;
	return half | (tile << 4) | y;
}

u16 ppu_spr2_addr(struct ppu* ppu, u8 tile, u8 y)
{
	u16 half = (tile & 1) << 12;
	return half | ((tile & ~1) << 4) | y;
}

int ppu_visible(struct ppu* ppu, u8 y)
{
	u8 yi = y + ((ppu->ctrl & C_SPR_SZ) ? 16 : 8);
	return ppu->scanline >= y && ppu->scanline < yi;
}

int ppu_horz_visible(struct ppu* ppu, u8 s)
{
	u8 cy = ppu->dot - 1;
	return ppu->sprites[s].x <= cy && cy < ppu->sprites[s].x + 8;
}

void ppu_ppudata_inc(struct ppu* ppu)
{
	if ((ppu->scanline < 240 || ppu->scanline == 261) && REND) {
		ppu_inc_x(ppu);
		ppu_inc_y(ppu);
	}
	else ppu->v += (ppu->ctrl & C_VRAM_INC) ? 32 : 1;
}

u8 ppu_io_read(struct ppu* ppu, u16 addr)
{
	u8 s;
	
	addr = 0x2000 | (addr % 8);

	switch (addr) {
	case 0x2000: case 0x2001: case 0x2003: case 0x2005: case 0x2006:
		return ppu->open_bus;

	case 0x2002:
		s = ppu->status;

		if (ppu->scanline == 241) {
			switch (ppu->dot) {
			case 1: ppu->vbl_block = 1; break;
			case 2: ppu->vbl_block = 1; nes.cpu.nmi = 0; break;
			case 3: nes.cpu.nmi = 0; break;
			}
		}

		ppu->status &= ~S_VBL;
		ppu->w = 0;
		return s | (ppu->open_bus & 0x1f);
	case 0x2004: 
		s = ((u8*)ppu->oam)[ppu->oam_addr];
		ppu_update_open_bus(ppu, s);
		return s;
	case 0x2007:
		if (ppu->v >= 0x3f00 && ppu->v <= 0x3fff) {
			s = ppu_read(ppu, ppu->v);
			/*wtf is this?*/
			ppu->ppudata_read_buffer = ppu->vram[ppu_mirror(ppu, ppu->v - 0x3000)];
		}
		else {
			s = ppu->ppudata_read_buffer;
			ppu->ppudata_read_buffer = ppu_read(ppu, ppu->v);
		}

		ppu_update_open_bus(ppu, s);
		ppu_ppudata_inc(ppu);

		return s;
	}

	return 0;
}

void ppu_io_write(struct ppu* ppu, u16 addr, u8 val)
{
	int i = 0;

	if((addr & 0x2000) == 0x2000)
		addr = 0x2000 | (addr % 8);

	ppu_update_open_bus(ppu, val);

	switch (addr) {
	case 0x2000: 
		if (!(ppu->ctrl & C_NMI) && (ppu->status & S_VBL) && (val & C_NMI) && (ppu->dot != 1))
			nes.cpu.nmi = nes.cpu.nmi_wait = 1;
		
		ppu->ctrl = val;
		ppu->t &= ~0xc00;
		ppu->t |= (val & 3) << 10;
		break;
	case 0x2001: 
		ppu->mask = val; 
		break;
	case 0x2003: 
		ppu->oam_addr = val;
		break;
	case 0x2004: 
		if (!(ppu->scanline < 240 || ppu->scanline == 261) || !REND) {
			if ((ppu->oam_addr & 3) == 2) val &= 0xe3;
			((u8*)ppu->oam)[ppu->oam_addr++] = val;
		}
		else ppu->oam_addr += 4;

		break;
	case 0x2005: 
		if (ppu->w) {
			ppu->t &= ~0x73e0;
			ppu->t |= (val & 7) << 12;
			ppu->t |= (val & 0xf8) << 2;
		}
		else {
			ppu->t &= ~0x1f;
			ppu->t |= val >> 3;
			ppu->x = val & 7;
		}

		ppu->w = !ppu->w;
		ppu->scroll = val;
		break;
	case 0x2006: 
		if (ppu->w) {
			ppu->t &= ~0xff;
			ppu->t |= val;
			ppu->v = ppu->t;
		}
		else {
			ppu->t &= ~0x3f00;
			ppu->t |= (val & 0x3f) << 8;
		}

		ppu->w = !ppu->w;
		break;
	case 0x2007: 
		ppu->ppu_data = val;
		ppu_write(ppu, ppu->v & 0x3fff, ppu->ppu_data); 
		ppu_ppudata_inc(ppu);
		break;
	case 0x4014: 
		ppu->oam_dma_addr = val;
		for (; i < 0x100; i++) {
			((u8*)ppu->oam)[i & 0xff] = nes_read((ppu->oam_dma_addr << 8) | i);
			C;
		}

		C; C; C; // 514 cycles

		break;
	}
}

void ppu_pixel(struct ppu* ppu)
{
#define P(k) ((ppu->k >> (15 - ppu->x)) & 1)
#define Q(z) ((z##hi << 1) | z##lo)
	u8 phi = P(pths), plo = P(ptls), ahi = P(aths), alo = P(atls), pt = Q(p), at = Q(a); // bg data
	u8 shi, slo;
	u16 spt = 0, sat = 0;
	u8 shift;
#undef P
	u8 bgcol = 0, // bg final pixel
		spcol = 0, // spr final pixel
		col = 0, // final pixel
		un = ppu_read(ppu, 0x3f00),
		cy = ppu->dot - 1; // cycle
	col = un;
#define P(k) ((ppu->sprites[ppu->spr].k >> shift) & 1)

	if (!(ppu->mask & M_BG) || (!(ppu->mask & M_BG_LM) && cy < 8)) {
		bgcol = un; pt = at = 0;
	}
	else {
		bgcol = pt;
		if (bgcol) bgcol |= at << 2;
		bgcol = ppu_read(ppu, 0x3f00 + (REND ? bgcol : 0));
	}

	if (!(ppu->mask & M_SPR) || (!(ppu->mask & M_SPR_LM) && cy < 8)) {
		spcol = un; spt = sat = 0;
	}
	else {
		for (ppu->spr = 0; ppu->spr < ppu->sprites_found; ppu->spr++) {
			if (ppu_horz_visible(ppu, ppu->spr)) {
				shift = 7 - (cy - ppu->sprites[ppu->spr].x);
				sat = ppu->sprites[ppu->spr].attr;
				
				shi = P(pthi), slo = P(ptlo);
				spt = Q(s);
				
				if (spt == 0) continue;

				spcol = spt;
				if (spcol) spcol |= (sat & A_PAL) << 2;
				spcol = ppu_read(ppu, 0x3f10 + (REND ? spcol : 0));
				break;
			}
		}
	}

	if (!pt && !spt) { col = un; }
	else if (pt && !spt) { col = bgcol; }
	else if (!pt && spt) { col = spcol; }
	else {
		if (!(sat & A_PRIO)) col = spcol;
		else col = bgcol;

		if (ppu->spr == 0 && !(ppu->status & S_SPR0HIT) && cy != 255) {
			ppu->status |= S_SPR0HIT;
		}
	}

	PX(cy, ppu->scanline) = ppu_clr_pal[col % 64];
	ppu_shift_registers(ppu);
}

void ppu_atl_adjust(struct ppu* ppu)
{
	u8 s = ((ppu->v >> 4) & 4) | (ppu->v & 2);
	ppu->atl = (ppu->atl >> s) & 3;
}

void ppu_bg_fetch(struct ppu* ppu)
{
	switch (ppu->dot % 8) {
	case 1: 
		ppu->ntl = ppu_read(ppu, NMTA); 
		if(ppu->dot >= 9) 
			ppu_shift_update(ppu); 
		break;
	case 3: 
		ppu->atl = ppu_read(ppu, ATRA); 
		ppu_atl_adjust(ppu); 
		break;
	case 5: ppu->ptl = ppu_read(ppu, ppu_bg_addr(ppu, ppu->ntl)); break;
	case 7: ppu->pth = ppu_read(ppu, ppu_bg_addr(ppu, ppu->ntl) + 8); break;
	case 0: ppu_inc_x(ppu); break;
	}
}

void ppu_spr_eval(struct ppu* ppu)
{
	if (!REND) return;

	if (ppu->dot == 65) {
		ppu->in_range = ppu->s = 
			ppu->done = 0;
		ppu->n = (ppu->oam_addr >> 2) & 0x3f;
		ppu->m = ppu->oam_addr & 3;
	}

	if (ppu->dot == 256) {
		ppu->sprites_found = 0;
	}

	if (ppu->dot & 1) {
		ppu->copy = ((u8*)ppu->oam)[ppu->oam_addr];
	}
	else {
		if (!ppu->done) {
			if (!ppu->in_range && ppu_visible(ppu, ppu->copy))
				ppu->in_range = 1;

			if (ppu->s < 32) {
				((u8*)ppu->soam)[ppu->s] = ppu->copy;

				if (ppu->in_range) {
					ppu->m++;
					ppu->s++;
					if (ppu->m == 4) {
						ppu->in_range = ppu->m = 0;
						ppu->n = (ppu->n + 1) & 0x3f;
						if (!ppu->n) ppu->done = 1;
					}
				}
				else {
					ppu->n = (ppu->n + 1) & 0x3f;
					if (!ppu->n) ppu->done = 1;
				}
			}
			else {
				ppu->copy = ((u8*)ppu->soam)[ppu->s & 0x1f];

				if (ppu->in_range) {
					ppu->status |= S_SPR_OVR;
					ppu->m++;
					if (ppu->m == 4) {
						ppu->m = 0;
						ppu->n = (ppu->n + 1) & 0x3f;
					}

					if (ppu->ctr != 2) {
						ppu->ctr = 0;
						ppu->done = 1;
					}
					else ppu->ctr++;
				}
				else {
					ppu->n = (ppu->n + 1) & 0x3f;
					ppu->m++;
					ppu->m &= 3;

					if (!ppu->n) ppu->done = 1;
				}
			}

			ppu->oam_addr = (ppu->n << 2) | (ppu->m & 3);
		}
		else {
			ppu->n = (ppu->n + 1) & 0x3f;
			if (ppu->s >= 32) {
				ppu->copy = ((u8*)ppu->soam)[ppu->oam_addr & 0x1f];
			}
		}
	}
}

u8 _swap(u8 b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

void ppu_spr_fetch(struct ppu* ppu)
{
	u16 addr;
	int y;

	ppu->cur_spr = (ppu->dot - 264) / 8;

	if (ppu->cur_spr > ppu->sprites_found) return;
	if (!(ppu->mask & M_SPR)) return;

	ppu->sprites[ppu->cur_spr].y = ppu->soam[ppu->cur_spr].y_coord;
	ppu->sprites[ppu->cur_spr].x = ppu->soam[ppu->cur_spr].x_coord;
	ppu->sprites[ppu->cur_spr].attr = ppu->soam[ppu->cur_spr].attr;

	y = ppu->scanline - ppu->sprites[ppu->cur_spr].y;

	if (!(ppu->ctrl & C_SPR_SZ)) { // 8x8
		// out of bounds?
		if (y > 7 || y < 0) return;

		// is sprite flipped vertically?
		if (ppu->sprites[ppu->cur_spr].attr & A_FLIP_VERT) {
			y = 7 - y;
		}

		addr = ppu_spr_addr(ppu, ppu->soam[ppu->cur_spr].tile, y);
	}
	else { // 8x16
		// out of bounds?
		if (y > 15 || y < 0) return;

		// is sprite flipped vertically?
		if (ppu->sprites[ppu->cur_spr].attr & A_FLIP_VERT) {
			y = 15 - y;
		}

		addr = ppu_spr2_addr(ppu, ppu->soam[ppu->cur_spr].tile, y);
	}

	ppu->sprites_found++;

	ppu->sprites[ppu->cur_spr].ptlo = ppu_read(ppu, addr);
	ppu->sprites[ppu->cur_spr].pthi = ppu_read(ppu, addr + 8);

	// is sprite flipped horizontally?
	if (ppu->sprites[ppu->cur_spr].attr & A_FLIP_HORZ) {
		ppu->sprites[ppu->cur_spr].ptlo = _swap(ppu->sprites[ppu->cur_spr].ptlo);
		ppu->sprites[ppu->cur_spr].pthi = _swap(ppu->sprites[ppu->cur_spr].pthi);
	}
}

void ppu_visible_tick(struct ppu* ppu)
{
	if (!REND) return;

	// reset internal x counter and 2nd OAM counter
	if (ppu->dot == 0) {
		ppu->s = ppu->done = 0;
	}
	// initialize secondary oam
	if (ppu->dot >= 1 && ppu->dot <= 64) {
		memset(ppu->soam, 0xff, 32);
	}
	// sprite evaluation for this line
	if (ppu->dot >= 65 && ppu->dot <= 256) {
		ppu_spr_eval(ppu);
	}
	// sprite loading
	if (ppu->dot >= 257 && ppu->dot <= 320) {
		ppu->oam_addr = 0;
		if (ppu->dot % 8 == 0)
			ppu_spr_fetch(ppu);
	}
	// background evaluation for this line and the next line
	if ((ppu->dot >= 1 && ppu->dot <= 256)) {
		ppu_bg_fetch(ppu);
		ppu_pixel(ppu);
	}
	else if ((ppu->dot >= 321 && ppu->dot <= 337)) {
		ppu_shift_registers(ppu);
		ppu_bg_fetch(ppu);
	}
	// increment vert(v)
	if (ppu->dot == 256) {
		ppu_shift_registers(ppu);
		ppu_inc_y(ppu);
	}
	// hori(v) = hori(t)
	if (ppu->dot == 257) {
		ppu_shift_registers(ppu);
		ppu_shift_update(ppu);
		ppu_horz_copy(ppu);
	}
}

void ppu_idle_tick(struct ppu* ppu)
{
	
}

void ppu_vblank_tick(struct ppu* ppu)
{
	if (ppu->scanline == 240 && ppu->dot == 0) {
		if (REND) window_draw(&nes.window);
	}

	if (ppu->scanline == 241 && ppu->dot == 1 && !ppu->vbl_block) {
		ppu->status |= S_VBL;
		ppu->ppu_delay = 3;
	}
}

void ppu_pre_rend_tick(struct ppu* ppu)
{
	ppu_pixel(ppu);

	if (ppu->dot == 0) {
		ppu->s = ppu->done = 0;
	}
	// initialize secondary oam
	if (ppu->dot >= 1 && ppu->dot <= 64) {
		memset(ppu->soam, 0xff, 32);
	}
	// sprite evaluation for next line
	if (ppu->dot >= 65 && ppu->dot <= 256) {
		ppu_spr_eval(ppu);
	}
	// vert(v) = vert(t)
	if (ppu->dot >= 280 && ppu->dot <= 304) {
		ppu_vert_copy(ppu);
	}
	// bg fetch for next line
	if (ppu->dot >= 321 && ppu->dot <= 337) {
		ppu_shift_registers(ppu);
		ppu_bg_fetch(ppu);
	}

	switch (ppu->dot) {
	case 1: 
		ppu->status &= ~(S_VBL | S_SPR0HIT | S_SPR_OVR); 
		break;
	case 338: 
		if ((ppu->frames & 1) && REND)
			ppu->dot++;
		break;
	}
}

void ppu_tick(struct ppu* ppu)
{
	if (ppu->scanline <= 239) ppu_visible_tick(ppu);
	else if (ppu->scanline >= 240 && ppu->scanline <= 260) ppu_vblank_tick(ppu);
	else if (ppu->scanline == 261) ppu_pre_rend_tick(ppu);

	if (ppu->ppu_delay) {
		if (!--ppu->ppu_delay && (ppu->ctrl & C_NMI) && (ppu->status & S_VBL)) {
			nes.cpu.nmi = 1;
		}
	}

	if (ppu->open_bus_decay) {
		if (!--ppu->open_bus_decay) ppu->open_bus = 0;
	}

	if (++ppu->dot > 340) {
 		ppu->dot = 0;
		if (++ppu->scanline > 261) {
			ppu->scanline = ppu->vbl_block = 0;
			++ppu->frames;
		}
	}
}