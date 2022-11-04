#include "cpu.h"

/*addressing modes*/

void IMM(struct cpu* cpu) { cpu->addr = cpu->PC++; }
void IMP(struct cpu* cpu) { C; }
void ACC(struct cpu* cpu) { C; }
void ABS(struct cpu* cpu) { cpu->addr = nes_read16(cpu->PC); cpu->PC += 2; }
void ABX(struct cpu* cpu) { cpu->addr = nes_read16(cpu->PC); cpu->PC += 2; if (CROSS(cpu->addr, cpu->X)) C; cpu->addr += cpu->X; }
void ABXW(struct cpu* cpu) { cpu->addr = nes_read16(cpu->PC); cpu->PC += 2; cpu->addr += cpu->X; C; }
void ABY(struct cpu* cpu) { cpu->addr = nes_read16(cpu->PC); cpu->PC += 2; if (CROSS(cpu->addr, cpu->Y)) C; cpu->addr += cpu->Y; }
void ABYW(struct cpu* cpu) { cpu->addr = nes_read16(cpu->PC); cpu->PC += 2; cpu->addr += cpu->Y; C; }
void REL(struct cpu* cpu) { cpu->dat = nes_read(cpu->PC++); cpu->addr = cpu->PC + (s8)cpu->dat; }
void ZPG(struct cpu* cpu) { cpu->addr = nes_read(cpu->PC++); }
void ZPX(struct cpu* cpu) { ZPG(cpu); cpu->addr = (cpu->addr + cpu->X) & 0xff; C; }
void ZPY(struct cpu* cpu) { ZPG(cpu); cpu->addr = (cpu->addr + cpu->Y) & 0xff; C; }
void IND(struct cpu* cpu) { ABS(cpu); cpu->addr = nes_read(cpu->addr) | (nes_read((cpu->addr & 0xff00) | ((cpu->addr + 1) & 0xff)) << 8); }
void IDX(struct cpu* cpu) { cpu->dat = nes_read(cpu->PC++); cpu->addr = nes_read((cpu->dat + cpu->X) & 0xff) | (nes_read((cpu->dat + cpu->X + 1) & 0xff) << 8); C; }
void IDY(struct cpu* cpu) { cpu->dat = nes_read(cpu->PC++); cpu->addr = nes_read(cpu->dat & 0xff) | (nes_read((cpu->dat + 1) & 0xff) << 8); if (CROSS(cpu->addr, cpu->Y)) C; cpu->addr += cpu->Y; }
void IDYW(struct cpu* cpu) { cpu->dat = nes_read(cpu->PC++); cpu->addr = nes_read(cpu->dat & 0xff) | (nes_read((cpu->dat + 1) & 0xff) << 8); cpu->addr += cpu->Y; C; }

/**/
void BR(struct cpu* cpu, u8 c) { if (c) { if (CROSS(cpu->PC, (s8)cpu->dat)) C; cpu->PC = cpu->addr; C; } }

/*instructions*/
void _ADC(struct cpu* cpu, u8 dat) 
{ 
	s16 result = cpu->A + dat + !!ISF(CF);
	TOGIF(CF, (result > 0xff));
	TOGIF(VF, ~(cpu->A ^ dat) & (cpu->A ^ result) & 0x80);
	cpu->A = (u8)result;
	TOGZN(cpu->A);
}
void ADC(struct cpu* cpu) { _ADC(cpu, (DAT)); }
void AND(struct cpu* cpu) { DAT; cpu->A &= cpu->dat; TOGZN(cpu->A); }
void ASLA(struct cpu* cpu) { TOGIF(CF, MSB(cpu->A)); cpu->A <<= 1; TOGZN(cpu->A); }
void ASLM(struct cpu* cpu) { DAT; TOGIF(CF, MSB(cpu->dat)); cpu->dat <<= 1; TOGZN(cpu->dat); nes_write(cpu->addr, cpu->dat); C; }
void BCC(struct cpu* cpu) { BR(cpu, !ISF(CF)); }
void BCS(struct cpu* cpu) { BR(cpu, ISF(CF)); }
void BEQ(struct cpu* cpu) { BR(cpu, ISF(ZF)); }
void BIT(struct cpu* cpu) 
{
	DAT;
	TOGIF(NF, cpu->dat & 0x80);
	TOGIF(VF, cpu->dat & 0x40);
	TOGIF(ZF, !(cpu->A & cpu->dat));
}
void BMI(struct cpu* cpu) { BR(cpu, ISF(NF)); }
void BNE(struct cpu* cpu) { BR(cpu, !ISF(ZF)); }
void BPL(struct cpu* cpu) { BR(cpu, !ISF(NF)); }
void BRK(struct cpu* cpu) { cpu_interrupt(cpu, I_BRK); }
void BVC(struct cpu* cpu) { BR(cpu, !ISF(VF)); }
void BVS(struct cpu* cpu) { BR(cpu, ISF(VF)); }
void CLC(struct cpu* cpu) { CLRF(CF); }
void CLD(struct cpu* cpu) { CLRF(DF); }
void CLI(struct cpu* cpu) { CLRF(IF); }
void CLV(struct cpu* cpu) { CLRF(VF); }
void CMP(struct cpu* cpu) { CP(cpu->A, cpu->dat); }
void CPX(struct cpu* cpu) { CP(cpu->X, cpu->dat); }
void CPY(struct cpu* cpu) {	CP(cpu->Y, cpu->dat); }
void DEC(struct cpu* cpu) { DAT; C; --cpu->dat; TOGZN(cpu->dat); nes_write(cpu->addr, cpu->dat); }
void DEX(struct cpu* cpu) { --cpu->X; TOGZN(cpu->X); }
void DEY(struct cpu* cpu) { --cpu->Y; TOGZN(cpu->Y); }
void EOR(struct cpu* cpu) { DAT; cpu->A ^= cpu->dat; TOGZN(cpu->A); }
void INC(struct cpu* cpu) { DAT; C; ++cpu->dat; TOGZN(cpu->dat); nes_write(cpu->addr, cpu->dat); }
void INX(struct cpu* cpu) { ++cpu->X; TOGZN(cpu->X); }
void INY(struct cpu* cpu) { ++cpu->Y; TOGZN(cpu->Y); }
void JMP(struct cpu* cpu) { cpu->PC = cpu->addr; }
void JSR(struct cpu* cpu) { C; push16(cpu, cpu->PC - 1); cpu->PC = cpu->addr; }
void LDA(struct cpu* cpu) { DAT; cpu->A = cpu->dat; TOGZN(cpu->A); }
void LDX(struct cpu* cpu) { DAT; cpu->X = cpu->dat; TOGZN(cpu->X); }
void LDY(struct cpu* cpu) { DAT; cpu->Y = cpu->dat; TOGZN(cpu->Y); }
void LSRA(struct cpu* cpu) 
{
	CLRF(NF);
	TOGIF(CF, cpu->A & 1);
	cpu->A >>= 1;
	TOGIF(ZF, !cpu->A);
}
void LSRM(struct cpu* cpu) 
{
	DAT;
	CLRF(NF);
	TOGIF(CF, cpu->dat & 1);
	cpu->dat >>= 1;
	TOGIF(ZF, !cpu->dat);
	C;
	nes_write(cpu->addr, cpu->dat);
}
void NOP(struct cpu* cpu) { (void)(cpu); }
void ORA(struct cpu* cpu) { DAT; cpu->A |= cpu->dat; TOGZN(cpu->A); }
void PHA(struct cpu* cpu) { push(cpu, cpu->A); }
void PHP(struct cpu* cpu) { push(cpu, cpu->P | BF); }
void PLA(struct cpu* cpu) { C; cpu->A = pop(cpu); TOGZN(cpu->A); }
void PLP(struct cpu* cpu) { C; cpu->P = pop(cpu); cpu->P &= ~BF; cpu->P |= (1 << 5); }
void ROLA(struct cpu* cpu) 
{
	u8 c = !!ISF(CF);
	TOGIF(CF, MSB(cpu->A));
	cpu->A = (cpu->A << 1) | c;
	TOGZN(cpu->A);
}
void ROLM(struct cpu* cpu) 
{
	u8 c = !!ISF(CF);
	DAT;
	TOGIF(CF, MSB(cpu->dat));
	cpu->dat = (cpu->dat << 1) | c;
	TOGZN(cpu->dat);
	C;
	nes_write(cpu->addr, cpu->dat);
}
void RORA(struct cpu* cpu) 
{
	u8 c = !!ISF(CF) << 7;
	TOGIF(CF, cpu->A & 1);
	cpu->A = (cpu->A >> 1) | c;
	TOGZN(cpu->A);
}
void RORM(struct cpu* cpu) 
{
	u8 c = !!ISF(CF) << 7;
	DAT;
	TOGIF(CF, cpu->dat & 1);
	cpu->dat = (cpu->dat >> 1) | c;
	TOGZN(cpu->dat);
	C;
	nes_write(cpu->addr, cpu->dat);
}
void RTI(struct cpu* cpu) { C; cpu->P = pop(cpu) | (1<<5); cpu->PC = pop16(cpu); }
void RTS(struct cpu* cpu) { C; cpu->PC = pop16(cpu) + 1; C; }
void SBC(struct cpu* cpu) { DAT; cpu->dat = ~cpu->dat; _ADC(cpu, cpu->dat); }
void SEC(struct cpu* cpu) { SETF(CF); }
void SED(struct cpu* cpu) { SETF(DF); }
void SEI(struct cpu* cpu) { SETF(IF); }
void STA(struct cpu* cpu) { nes_write(cpu->addr, cpu->A); }
void STX(struct cpu* cpu) { nes_write(cpu->addr, cpu->X); }
void STY(struct cpu* cpu) { nes_write(cpu->addr, cpu->Y); }
void TAX(struct cpu* cpu) { cpu->X = cpu->A; TOGZN(cpu->X); }
void TAY(struct cpu* cpu) { cpu->Y = cpu->A; TOGZN(cpu->Y); }
void TSX(struct cpu* cpu) { cpu->X = cpu->S; TOGZN(cpu->X); }
void TXA(struct cpu* cpu) { cpu->A = cpu->X; TOGZN(cpu->A); }
void TXS(struct cpu* cpu) { cpu->S = cpu->X; }
void TYA(struct cpu* cpu) { cpu->A = cpu->Y; TOGZN(cpu->A); }

/*illegal instructions*/
void ALR(struct cpu* cpu) { AND(cpu); LSRA(cpu); }
void ANC(struct cpu* cpu) { AND(cpu); TOGIF(CF, MSB(cpu->A)); }
void ANE(struct cpu* cpu) { cpu->A = cpu->A & cpu->X & (DAT); TOGZN(cpu->A); }
void ARR(struct cpu* cpu) { cpu->A &= (DAT); cpu->A = (cpu->A >> 1) | (!!ISF(CF) << 7); TOGZN(cpu->A); TOGIF(CF, cpu->A & 0x40); TOGIF(VF, !!ISF(CF) ^ !!(cpu->A & 0x20)); }
void DCP(struct cpu* cpu) { DEC(cpu); _CP(cpu->A, cpu->dat); }
void ISC(struct cpu* cpu) { INC(cpu); cpu->dat = ~cpu->dat; _ADC(cpu, cpu->dat); }
void LAS(struct cpu* cpu) { DAT; cpu->dat &= cpu->S; cpu->A = cpu->X = cpu->S = cpu->dat; }
void LAX(struct cpu* cpu) { LDA(cpu); TAX(cpu); }
void LXA(struct cpu* cpu) { cpu->A = (DAT); TAX(cpu); }
void RLA(struct cpu* cpu) { ROLM(cpu); cpu->A &= cpu->dat; TOGZN(cpu->A); }
void RRA(struct cpu* cpu) { RORM(cpu); _ADC(cpu, cpu->dat); }
void SAX(struct cpu* cpu) { nes_write(cpu->addr, cpu->A & cpu->X); }
void SBX(struct cpu* cpu) { s16 t = (cpu->X & cpu->A) - (DAT); cpu->X = (u8)t; TOGZN(cpu->X); TOGIF(CF, t >= 0); }
void SHA(struct cpu* cpu) { nes_write(cpu->addr, cpu->A & cpu->X & ((cpu->addr >> 8) + 1)); }
void SHX(struct cpu* cpu) { u16 a = cpu->PC - 2; u8 v = cpu->X & ((cpu->addr >> 8) + 1); if (cpu->Y + nes_read(a) <= 0xff) nes_write(cpu->addr, v); }
void SHY(struct cpu* cpu) { u16 a = cpu->PC - 2; u8 v = cpu->Y & ((cpu->addr >> 8) + 1); if (cpu->X + nes_read(a) <= 0xff) nes_write(cpu->addr, v); }
void SLO(struct cpu* cpu) { ASLM(cpu); cpu->A |= cpu->dat; TOGZN(cpu->A); }
void SRE(struct cpu* cpu) { LSRM(cpu); cpu->A ^= cpu->dat; TOGZN(cpu->A); }
void TAS(struct cpu* cpu) { cpu->S = cpu->A & cpu->X; nes_write(cpu->addr, cpu->S & ((cpu->addr >> 8) + 1)); }
void JAM(struct cpu* cpu) { puts("CPU JAMMED"); cpu->halted = 1; cpu->dat = 0xff; }
void UNOP(struct cpu* cpu) { DAT; }

void push(struct cpu* cpu, u8 dat)
{
	nes_write(0x100 | cpu->S--, dat);
}

u8 pop(struct cpu* cpu)
{
	return nes_read(0x100 | ++cpu->S);
}

void push16(struct cpu* cpu, u16 dat)
{
	push(cpu, dat >> 8);
	push(cpu, dat & 0xff);
}

u16 pop16(struct cpu* cpu)
{
	u16 res = pop(cpu);
	res |= (pop(cpu) << 8);
	return res;
}

void cpu_reset(struct cpu* cpu)
{
	cpu->A = cpu->X = cpu->Y = cpu->S = 0;
	cpu->P = 0x04;
	cpu->nmi = cpu->irq = 0;
	cpu_interrupt(cpu, I_RES);
}

void cpu_interrupt(struct cpu* cpu, u8 interrupt)
{
	switch (interrupt) {
	case I_BRK:
		push16(cpu, ++cpu->PC);
		push(cpu, cpu->P | BF);
		SETF(IF);
		cpu->PC = nes_read16(0xfffe);
		break;
	case I_RES:
		C; C; C;
		cpu->S -= 3; C; C; C;
		cpu->PC = nes_read16(0xfffc);
		break;
	case I_NMI:
	case I_IRQ:
		if (cpu->nmi) {
			if (cpu->nmi_wait) {
				cpu->nmi_wait = 0;
				break;
			}

			cpu->nmi = 0;
		}
		else {
			cpu->irq = 0;
		}

		C; C;
		push16(cpu, cpu->PC);
		push(cpu, cpu->P & ~BF);
		SETF(IF);
		cpu->PC = nes_read16(0xfffa + interrupt);
		break;
	}
}

void cpu_step(struct cpu* cpu)
{
	if (cpu->halted) return;

	if (cpu->nmi || (cpu->irq && !ISF(IF))) {
		if (cpu->nmi)
			cpu_interrupt(cpu, I_NMI);
		else
			cpu_interrupt(cpu, I_IRQ);
	}

	cpu->op = nes_read(cpu->PC++);
	cpu->addr = cpu->dat = cpu->pgc = 0;
	//disas_print_state(cpu);

#define o(h,a,op) case 0x##h: a(cpu); op(cpu); break;
	switch (cpu->op) {
		o(00, IMP, BRK)	o(01, IDX, ORA)	o(02, IMP, JAM)	o(03, IDX, SLO)
		o(04, ZPG,UNOP)	o(05, ZPG, ORA)	o(06, ZPG, ASLM)o(07, ZPG, SLO)
		o(08, IMP, PHP)	o(09, IMM, ORA)	o(0a, ACC, ASLA)o(0b, IMM, ANC)
		o(0c, ABS,UNOP)	o(0d, ABS, ORA)	o(0e, ABS, ASLM)o(0f, ABS, SLO)
		o(10, REL, BPL)	o(11, IDY, ORA)	o(12, IMP, JAM)	o(13, IDYW,SLO)
		o(14, ZPX,UNOP)	o(15, ZPX, ORA)	o(16, ZPX, ASLM)o(17, ZPX, SLO)
		o(18, IMP, CLC)	o(19, ABY, ORA)	o(1a, IMP, NOP)	o(1b, ABYW,SLO)
		o(1c, ABX,UNOP)	o(1d, ABX, ORA)	o(1e, ABXW,ASLM)o(1f, ABXW,SLO)
		o(20, ABS, JSR)	o(21, IDX, AND)	o(22, IMP, JAM)	o(23, IDX, RLA)
		o(24, ZPG, BIT)	o(25, ZPG, AND)	o(26, ZPG, ROLM)o(27, ZPG, RLA)
		o(28, IMP, PLP)	o(29, IMM, AND)	o(2a, ACC, ROLA)o(2b, IMM, ANC)
		o(2c, ABS, BIT)	o(2d, ABS, AND)	o(2e, ABS, ROLM)o(2f, ABS, RLA)
		o(30, REL, BMI)	o(31, IDY, AND)	o(32, IMP, JAM)	o(33, IDYW,RLA)
		o(34, ZPX,UNOP)	o(35, ZPX, AND)	o(36, ZPX, ROLM)o(37, ZPX, RLA)
		o(38, IMP, SEC)	o(39, ABY, AND)	o(3a, IMP, NOP)	o(3b, ABYW,RLA)
		o(3c, ABX,UNOP)	o(3d, ABX, AND)	o(3e, ABXW,ROLM)o(3f, ABXW,RLA)
		o(40, IMP, RTI)	o(41, IDX, EOR)	o(42, IMP, JAM)	o(43, IDX, SRE)
		o(44, ZPG,UNOP)	o(45, ZPG, EOR)	o(46, ZPG, LSRM)o(47, ZPG, SRE)
		o(48, IMP, PHA)	o(49, IMM, EOR)	o(4a, ACC, LSRA)o(4b, IMM, ALR)
		o(4c, ABS, JMP)	o(4d, ABS, EOR)	o(4e, ABS, LSRM)o(4f, ABS, SRE)
		o(50, REL, BVC)	o(51, IDY, EOR)	o(52, IMP, JAM)	o(53, IDYW,SRE)
		o(54, ZPX,UNOP)	o(55, ZPX, EOR)	o(56, ZPX, LSRM)o(57, ZPX, SRE)
		o(58, IMP, CLI)	o(59, ABY, EOR)	o(5a, IMP, NOP)	o(5b, ABYW,SRE)
		o(5c, ABX,UNOP)	o(5d, ABX, EOR)	o(5e, ABXW,LSRM)o(5f, ABXW,SRE)
		o(60, IMP, RTS)	o(61, IDX, ADC)	o(62, IMP, JAM)	o(63, IDX, RRA)
		o(64, ZPG,UNOP)	o(65, ZPG, ADC)	o(66, ZPG, RORM)o(67, ZPG, RRA)
		o(68, IMP, PLA)	o(69, IMM, ADC)	o(6a, ACC, RORA)o(6b, IMM, ARR)
		o(6c, IND, JMP)	o(6d, ABS, ADC)	o(6e, ABS, RORM)o(6f, ABS, RRA)
		o(70, REL, BVS)	o(71, IDY, ADC)	o(72, IMP, JAM)	o(73, IDYW,RRA)
		o(74, ZPX,UNOP)	o(75, ZPX, ADC)	o(76, ZPX, RORM)o(77, ZPX, RRA)
		o(78, IMP, SEI)	o(79, ABY, ADC)	o(7a, IMP, NOP)	o(7b, ABYW,RRA)
		o(7c, ABX,UNOP)	o(7d, ABX, ADC)	o(7e, ABXW,RORM)o(7f, ABXW,RRA)
		o(80, IMM,UNOP)	o(81, IDX, STA)	o(82, IMM, UNOP)o(83, IDX, SAX)
		o(84, ZPG, STY)	o(85, ZPG, STA)	o(86, ZPG, STX)	o(87, ZPG, SAX)
		o(88, IMP, DEY)	o(89, IMM,UNOP)	o(8a, IMP, TXA)	o(8b, IMM, ANE)
		o(8c, ABS, STY)	o(8d, ABS, STA)	o(8e, ABS, STX)	o(8f, ABS, SAX)
		o(90, REL, BCC)	o(91, IDYW,STA) o(92, IMP, JAM) o(93, IDYW,SHA)
		o(94, ZPX, STY)	o(95, ZPX, STA)	o(96, ZPY, STX)	o(97, ZPY, SAX)
		o(98, IMP, TYA)	o(99, ABYW,STA) o(9a, IMP, TXS)	o(9b, ABY, TAS)
		o(9c, ABXW,SHY)	o(9d, ABXW,STA) o(9e, ABYW,SHX) o(9f, ABYW,SHA)
		o(a0, IMM, LDY)	o(a1, IDX, LDA)	o(a2, IMM, LDX)	o(a3, IDX, LAX)
		o(a4, ZPG, LDY)	o(a5, ZPG, LDA)	o(a6, ZPG, LDX)	o(a7, ZPG, LAX)
		o(a8, IMP, TAY)	o(a9, IMM, LDA)	o(aa, IMP, TAX)	o(ab, IMM, LXA)
		o(ac, ABS, LDY)	o(ad, ABS, LDA)	o(ae, ABS, LDX)	o(af, ABS, LAX)
		o(b0, REL, BCS)	o(b1, IDY, LDA)	o(b2, IMP, JAM)	o(b3, IDY, LAX)
		o(b4, ZPX, LDY) o(b5, ZPX, LDA)	o(b6, ZPY, LDX)	o(b7, ZPY, LAX)
		o(b8, IMP, CLV)	o(b9, ABY, LDA)	o(ba, IMP, TSX)	o(bb, ABY, LAS)
		o(bc, ABX, LDY)	o(bd, ABX, LDA)	o(be, ABY, LDX)	o(bf, ABY, LAX)
		o(c0, IMM, CPY)	o(c1, IDX, CMP)	o(c2, IMM,UNOP)	o(c3, IDX, DCP)
		o(c4, ZPG, CPY)	o(c5, ZPG, CMP)	o(c6, ZPG, DEC)	o(c7, ZPG, DCP)
		o(c8, IMP, INY)	o(c9, IMM, CMP)	o(ca, IMP, DEX)	o(cb, IMM, SBX)
		o(cc, ABS, CPY)	o(cd, ABS, CMP)	o(ce, ABS, DEC)	o(cf, ABS, DCP)
		o(d0, REL, BNE)	o(d1, IDY, CMP)	o(d2, IMP, JAM)	o(d3, IDYW,DCP)
		o(d4, ZPX,UNOP)	o(d5, ZPX, CMP)	o(d6, ZPX, DEC)	o(d7, ZPX, DCP)
		o(d8, IMP, CLD)	o(d9, ABY, CMP)	o(da, IMP, NOP)	o(db, ABYW,DCP)
		o(dc, ABX,UNOP)	o(dd, ABX, CMP)	o(de, ABXW,DEC) o(df, ABXW,DCP)
		o(e0, IMM, CPX)	o(e1, IDX, SBC)	o(e2, IMM,UNOP)	o(e3, IDX, ISC)
		o(e4, ZPG, CPX)	o(e5, ZPG, SBC)	o(e6, ZPG, INC)	o(e7, ZPG, ISC)
		o(e8, IMP, INX)	o(e9, IMM, SBC)	o(ea, IMP, NOP)	o(eb, IMM, SBC)
		o(ec, ABS, CPX)	o(ed, ABS, SBC)	o(ee, ABS, INC)	o(ef, ABS, ISC)
		o(f0, REL, BEQ)	o(f1, IDY, SBC)	o(f2, IMP, JAM)	o(f3, IDYW,ISC)
		o(f4, ZPX,UNOP)	o(f5, ZPX, SBC)	o(f6, ZPX, INC)	o(f7, ZPX, ISC)
		o(f8, IMP, SED)	o(f9, ABY, SBC)	o(fa, IMP, NOP)	o(fb, ABYW,ISC)
		o(fc, ABX,UNOP)	o(fd, ABX, SBC)	o(fe, ABXW, INC)o(ff, ABXW,ISC)
	}
#undef o
}