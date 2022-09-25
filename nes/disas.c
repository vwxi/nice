#include "cpu.h"
#include "disas.h"

enum adm {
	a_ABS,
	a_ABX,
	a_ABY,
	a_IMM,
	a_IND,
	a_IDX,
	a_IDY,
	a_REL,
	a_ZPG,
	a_ZPX,
	a_ZPY,
	a_IMP,
	a_ACC
};

struct entry {
	char* name;
	enum adm mode;
};

const struct entry entries[256] = {
#define o(am,op) {#op, a_##am},
		o(IMP, BRK)	o(IDX, ORA)	o(IMP, JAM)	o(IDX, SLO)
		o(ZPG, NOP)	o(ZPG, ORA)	o(ZPG, ASL) o(ZPG, SLO)
		o(IMP, PHP)	o(IMM, ORA)	o(ACC, ASL) o(IMM, ANC)
		o(ABS, NOP)	o(ABS, ORA)	o(ABS, ASL) o(ABS, SLO)
		o(REL, BPL)	o(IDY, ORA)	o(IMP, JAM)	o(IDY, SLO)
		o(ZPX, NOP)	o(ZPX, ORA)	o(ZPX, ASL) o(ZPX, SLO)
		o(IMP, CLC)	o(ABY, ORA)	o(IMP, NOP)	o(ABY, SLO)
		o(ABX, NOP)	o(ABX, ORA)	o(ABX, ASL) o(ABX, SLO)
		o(ABS, JSR)	o(IDX, AND)	o(IMP, JAM)	o(IDX, RLA)
		o(ZPG, BIT)	o(ZPG, AND)	o(ZPG, ROL) o(ZPG, RLA)
		o(IMP, PLP)	o(IMM, AND)	o(ACC, ROL) o(IMM, ANC)
		o(ABS, BIT)	o(ABS, AND)	o(ABS, ROL) o(ABS, RLA)
		o(REL, BMI)	o(IDY, AND)	o(IMP, JAM)	o(IDY, RLA)
		o(ZPX, NOP)	o(ZPX, AND)	o(ZPX, ROL) o(ZPX, RLA)
		o(IMP, SEC)	o(ABY, AND)	o(IMP, NOP)	o(ABY, RLA)
		o(ABX, NOP)	o(ABX, AND)	o(ABX, ROL) o(ABX, RLA)
		o(IMP, RTI)	o(IDX, EOR)	o(IMP, JAM)	o(IDX, SRE)
		o(ZPG, NOP)	o(ZPG, EOR)	o(ZPG, LSR) o(ZPG, SRE)
		o(IMP, PHA)	o(IMM, EOR)	o(ACC, LSR) o(IMM, ALR)
		o(ABS, JMP)	o(ABS, EOR)	o(ABS, LSR) o(ABS, SRE)
		o(REL, BVC)	o(IDY, EOR)	o(IMP, JAM)	o(IDY, SRE)
		o(ZPX, NOP)	o(ZPX, EOR)	o(ZPX, LSR) o(ZPX, SRE)
		o(IMP, CLI)	o(ABY, EOR)	o(IMP, NOP)	o(ABY, SRE)
		o(ABX, NOP)	o(ABX, EOR)	o(ABX, LSR) o(ABX, SRE)
		o(IMP, RTS)	o(IDX, ADC)	o(IMP, JAM)	o(IDX, RRA)
		o(ZPG, NOP)	o(ZPG, ADC)	o(ZPG, ROR) o(ZPG, RRA)
		o(IMP, PLA)	o(IMM, ADC)	o(ACC, ROR) o(IMM, ARR)
		o(IND, JMP)	o(ABS, ADC)	o(ABS, ROR) o(ABS, RRA)
		o(REL, BVS)	o(IDY, ADC)	o(IMP, JAM)	o(IDY, RRA)
		o(ZPX, NOP)	o(ZPX, ADC)	o(ZPX, ROR) o(ZPX, RRA)
		o(IMP, SEI)	o(ABY, ADC)	o(IMP, NOP)	o(ABY, RRA)
		o(ABX, NOP)	o(ABX, ADC)	o(ABX, ROR) o(ABX, RRA)
		o(IMM, NOP)	o(IDX, STA)	o(IMM, NOP) o(IDX, SAX)
		o(ZPG, STY)	o(ZPG, STA)	o(ZPG, STX)	o(ZPG, SAX)
		o(IMP, DEY)	o(IMM, NOP)	o(IMP, TXA)	o(IMM, ANE)
		o(ABS, STY)	o(ABS, STA)	o(ABS, STX)	o(ABS, SAX)
		o(REL, BCC)	o(IDY, STA) o(IMP, JAM) o(IDY, SHA)
		o(ZPX, STY)	o(ZPX, STA)	o(ZPY, STX)	o(ZPY, SAX)
		o(IMP, TYA)	o(ABY, STA) o(IMP, TXS)	o(ABY, TAS)
		o(ABX, SHY)	o(ABX, STA) o(ABY, SHX) o(ABY, SHA)
		o(IMM, LDY)	o(IDX, LDA)	o(IMM, LDX)	o(IDX, LAX)
		o(ZPG, LDY)	o(ZPG, LDA)	o(ZPG, LDX)	o(ZPG, LAX)
		o(IMP, TAY)	o(IMM, LDA)	o(IMP, TAX)	o(IMM, LXA)
		o(ABS, LDY)	o(ABS, LDA)	o(ABS, LDX)	o(ABS, LAX)
		o(REL, BCS)	o(IDY, LDA)	o(IMP, JAM)	o(IDY, LAX)
		o(ZPX, LDY) o(ZPX, LDA)	o(ZPY, LDX)	o(ZPY, LAX)
		o(IMP, CLV)	o(ABY, LDA)	o(IMP, TSX)	o(ABY, LAS)
		o(ABX, LDY)	o(ABX, LDA)	o(ABY, LDX)	o(ABY, LAX)
		o(IMM, CPY)	o(IDX, CMP)	o(IMM, NOP)	o(IDX, DCP)
		o(ZPG, CPY)	o(ZPG, CMP)	o(ZPG, DEC)	o(ZPG, DCP)
		o(IMP, INY)	o(IMM, CMP)	o(IMP, DEX)	o(IMM, SBX)
		o(ABS, CPY)	o(ABS, CMP)	o(ABS, DEC)	o(ABS, DCP)
		o(REL, BNE)	o(IDY, CMP)	o(IMP, JAM)	o(IDY, DCP)
		o(ZPX, NOP)	o(ZPX, CMP)	o(ZPX, DEC)	o(ZPX, DCP)
		o(IMP, CLD)	o(ABY, CMP)	o(IMP, NOP)	o(ABY, DCP)
		o(ABX, NOP)	o(ABX, CMP)	o(ABX, DEC) o(ABX, DCP)
		o(IMM, CPX)	o(IDX, SBC)	o(IMM, NOP)	o(IDX, ISB)
		o(ZPG, CPX)	o(ZPG, SBC)	o(ZPG, INC)	o(ZPG, ISB)
		o(IMP, INX)	o(IMM, SBC)	o(IMP, NOP)	o(IMM, SBC)
		o(ABS, CPX)	o(ABS, SBC)	o(ABS, INC)	o(ABS, ISB)
		o(REL, BEQ)	o(IDY, SBC)	o(IMP, JAM)	o(IDY, ISB)
		o(ZPX, NOP)	o(ZPX, SBC)	o(ZPX, INC)	o(ZPX, ISB)
		o(IMP, SED)	o(ABY, SBC)	o(IMP, NOP)	o(ABY, ISB)
		o(ABX, NOP)	o(ABX, SBC)	o(ABX, INC) o(ABX, ISB)
#undef o
};

#define R16(a) nes_dbg_read(a) | (nes_dbg_read(a+1) << 8)

char* disas_fmt_opcode(struct cpu* cpu)
{
	/*unless i'm an idiot, 64 bytes should be enough for 6502 instructions*/
	char* fmt = malloc(64), *ptr = NULL;
	u16 a;
	u8 d;
	if (!fmt)
		return NULL;
	ptr = fmt;

	ptr += sprintf(ptr, "%s ", entries[cpu->op].name);
	
	switch (entries[cpu->op].mode) {
	case a_ABS:
		a = R16(cpu->PC);
		if (cpu->op != 0x20 && cpu->op != 0x4c) /*jsr and jmp*/
			ptr += sprintf(ptr, "$%04X = %02X", a, nes_dbg_read(a));
		else
			ptr += sprintf(ptr, "$%04X", a);
		break;
	case a_ABX:
		a = R16(cpu->PC);
		ptr += sprintf(ptr, "$%04X,X @ %04X = %02X", a, a + cpu->X, nes_dbg_read(a + cpu->X));
		break;
	case a_ABY:
		a = R16(cpu->PC);
		ptr += sprintf(ptr, "$%04X,Y @ %04X = %02X", a, a + cpu->Y, nes_dbg_read(a + cpu->Y));
		break;
	case a_IMM:
		d = nes_dbg_read(cpu->PC);
		ptr += sprintf(ptr, "#$%02X", d);
		break;
	case a_IND:
		a = R16(cpu->PC);
		ptr += sprintf(ptr, "($%04X) = ", a);
		a = R16(a);
		ptr += sprintf(ptr, "%04X", a);
		break;
	case a_IDX:
		d = nes_dbg_read(cpu->PC) + cpu->X;
		a = R16(d);
		ptr += sprintf(ptr, "($%02X,X) @ %02X = %04X = %02X", (u8)(d - cpu->X), d, a, nes_dbg_read(a));
		break;
	case a_IDY:
		d = nes_dbg_read(cpu->PC);
		a = R16(d);
		ptr += sprintf(ptr, "($%02X),Y = %04X @ %04X = %02X", d, a, a + cpu->Y, nes_dbg_read(a + cpu->Y));
		break;
	case a_REL:
		d = nes_dbg_read(cpu->PC);
		a = cpu->PC + 1 + (s8)d;
		ptr += sprintf(ptr, "$%04X", a);
		break;
	case a_ZPG:
		a = nes_dbg_read(cpu->PC);
		d = nes_dbg_read(a);
		ptr += sprintf(ptr, "$%02X = %02X", a, d);
		break;
	case a_ZPX:
		a = nes_dbg_read(cpu->PC);
		d = a + cpu->X;
		ptr += sprintf(ptr, "$%02X,X @ %02X = %02X", (u8)(d - cpu->X), d, nes_dbg_read(d));
		break;
	case a_ZPY:
		a = nes_dbg_read(cpu->PC);
		d = a + cpu->Y;
		ptr += sprintf(ptr, "$%02X,Y @ %02X = %02X", (u8)(d - cpu->Y), d, nes_dbg_read(d));
		break;
	case a_IMP:
		break;
	case a_ACC:
		ptr += sprintf(ptr, "A");
		break;
	}

	return fmt;
}

char* disas_op_bytes(struct cpu* cpu)
{
	/*no opcode needs any more than this*/
	char* fmt = malloc(16), *ptr = NULL;
	if (!fmt)
		return NULL;

	ptr = fmt;

	switch (entries[cpu->op].mode) {
	case a_IMP: case a_ACC:
		ptr += sprintf(ptr, "%02X", cpu->op);
		break;
	case a_IMM: case a_IDX: case a_IDY: case a_REL:
	case a_ZPG: case a_ZPX: case a_ZPY:
		ptr += sprintf(ptr, "%02X %02X", cpu->op, nes_dbg_read(cpu->PC));
		break;
	case a_ABS: case a_ABX: case a_ABY: case a_IND:
		ptr += sprintf(ptr, "%02X %02X %02X", cpu->op, nes_dbg_read(cpu->PC), nes_dbg_read(cpu->PC + 1));
		break;
	}

	return fmt;
}

/**/
void disas_print_state(struct cpu* cpu)
{
	char* fmt = disas_fmt_opcode(cpu),
		*bytes = disas_op_bytes(cpu);
	
	if (!fmt || !bytes) {
		puts("failed to format opcode and opcode bytes");
	}

	/*printf("%04X  %s\t%-32s"
		"A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d SL:%d\n", 
		cpu->PC - 1, bytes, fmt,
		cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, nes.ppu.dot, nes.ppu.scanline);*/

	printf("%04X %-32s"
		"A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d SL:%d\n",
		cpu->PC - 1, fmt,
		cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, nes.ppu.dot, nes.ppu.scanline);

	free(bytes);
	free(fmt);
}