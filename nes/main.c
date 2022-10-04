#include "nes.h"

int main(int argc, char** argv)
{
	FILE* fp = NULL;
	struct ines_header hdr;

	if (argc != 2) {
		printf("USAGE: %s [ROM file]\n", argv[0]);
		return 1;
	}

	if (!(fp = fopen(argv[1], "rb"))) {
		puts("failed to open file");
		goto bad;
	}

	if (!fread(&hdr, sizeof(struct ines_header), 1, fp)) {
		puts("couldn't read iNES header");
		goto bad;
	}

	if (!hdr.chr_sz)
		nes.cart.chr_ram =  1;

	nes.cart.prg_banks = hdr.prg_sz;
	nes.cart.chr_banks = hdr.chr_sz;
	nes.cart.prg_sz = 0x4000 * hdr.prg_sz;
	nes.cart.chr_sz = 0x2000 * (nes.cart.chr_ram ? 1 : hdr.chr_sz);
	nes.cart.prg_ram_sz = (hdr.ram_sz != 0) ? 0x2000 * hdr.ram_sz : 0x2000;
	nes.ppu.mirror = !(hdr.fl6 & (1 << 3)) ? hdr.fl6 & 1 : 2;
	nes.cart.type = hdr.fl6 >> 4;
	
	if (!nes_mapper_supported()) {
		puts("mapper unsupported");
		goto bad;
	}

	if (!nes_init()) {
		goto bad;
	}

	nes.cart.prg = malloc(nes.cart.prg_sz);
	if (!nes.cart.prg) {
		puts("failed to allocate PRG ROM to heap");
		goto bad;
	}

	nes.cart.chr = malloc(nes.cart.chr_sz);
	if (!nes.cart.chr) {
		puts("failed to allocate CHR ROM to heap");
		goto bad;
	}

	if (fread(nes.cart.prg, 1, nes.cart.prg_sz, fp) != nes.cart.prg_sz) {
		puts("failed to read PRG ROM");
		goto bad;
	}

	nes.cart.prg_ram = malloc(nes.cart.prg_ram_sz);
	if (!nes.cart.prg_ram) {
		puts("failed to allocate PRG RAM to heap");
		goto bad;
	}

	memset(nes.cart.prg_ram, 0, 0x1000);
	if (nes.cart.chr_sz && !nes.cart.chr_ram) {
		if (fread(nes.cart.chr, 1, nes.cart.chr_sz, fp) != nes.cart.chr_sz) {
			puts("failed to read CHR ROM");
			goto bad;
		}
	}

	cart_init(&nes.cart);

	printf("mapper %d, prg %dK (%d banks), chr %dK (%d banks), chr ram? %s\n",
		nes.cart.type, nes.cart.prg_sz / 1024, nes.cart.prg_banks, nes.cart.chr_sz / 1024, nes.cart.chr_banks,
		nes.cart.chr_ram ? "yes" : "no");

	cpu_reset(&nes.cpu);

	window_run(&nes.window);

bad:
	window_destroy(&nes.window);
	free(nes.cart.prg);
	free(nes.cart.chr);
	if(fp) fclose(fp);
	return 0;
}