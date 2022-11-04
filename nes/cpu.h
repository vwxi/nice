#pragma once

#include "def.h"
#include "disas.h"

#define I_NMI 0
#define I_BRK 1
#define I_RES 2
#define I_IRQ 4

#define C nes_cycle();

#define NF (1 << 7)
#define VF (1 << 6)
#define BF (1 << 4)
#define DF (1 << 3)
#define IF (1 << 2)
#define ZF (1 << 1)
#define CF (1 << 0)

#define ISF(f) (cpu->P & f)
#define SETF(f) cpu->P |= f
#define TOGF(f) cpu->P ^= f
#define CLRF(f) cpu->P &= ~(f)
#define CLRALL CLRF(NF|VF|DF|BF|IF|ZF|CF)

#define TOGIF(f,p) if(p) SETF(f); else CLRF(f);

#define MSB(b) (b & 0x80)

#define TOGZN(d) \
	TOGIF(ZF, !d); \
	TOGIF(NF, MSB(d)); \

#define CROSS(i,j) ((i + j) & 0xff00) != (i & 0xff00)

#define _CP(a,b) \
	TOGZN((a - b)); \
	TOGIF(CF, a >= b);

#define DAT cpu->dat = nes_read(cpu->addr)

#define CP(a,b) \
	DAT; \
	_CP(a,b);

struct cpu {
	u8 op;
	u16 addr;
	u8 dat;
	
	u8 ram[0x800];

	u16 PC;
	u8 S;
	u8 P;
	u8 A;
	u8 X;
	u8 Y;

	u8 nmi_wait;
	u8 nmi;

	u8 irq;

	u8 halted;
	u8 pgc;

	u64 cycles;

	struct cart* cart;
};

void push(struct cpu*, u8);
u8 pop(struct cpu*);
void push16(struct cpu*, u16);
u16 pop16(struct cpu*);

void cpu_interrupt(struct cpu*, u8);
void cpu_reset(struct cpu*);
void cpu_step(struct cpu*);

void ABS(struct cpu* cpu);
void ABX(struct cpu* cpu);
void ABXW(struct cpu* cpu);
void ABY(struct cpu* cpu);
void ABYW(struct cpu* cpu);
void IMM(struct cpu* cpu);
void IND(struct cpu* cpu);
void IDX(struct cpu* cpu);
void IDY(struct cpu* cpu);
void IDYW(struct cpu* cpu);
void REL(struct cpu* cpu);
void ZPG(struct cpu* cpu);
void ZPX(struct cpu* cpu);
void ZPY(struct cpu* cpu);
void IMP(struct cpu* cpu);
void ACC(struct cpu* cpu);

void BR(struct cpu* cpu, u8);

/*official instructions*/
void ADC(struct cpu* cpu);
void AND(struct cpu* cpu);
void ASLA(struct cpu* cpu);
void ASLM(struct cpu* cpu);
void BCC(struct cpu* cpu);
void BCS(struct cpu* cpu);
void BEQ(struct cpu* cpu);
void BIT(struct cpu* cpu);
void BMI(struct cpu* cpu);
void BNE(struct cpu* cpu);
void BPL(struct cpu* cpu);
void BRK(struct cpu* cpu);
void BVC(struct cpu* cpu);
void BVS(struct cpu* cpu);
void CLC(struct cpu* cpu);
void CLD(struct cpu* cpu);
void CLI(struct cpu* cpu);
void CLV(struct cpu* cpu);
void CMP(struct cpu* cpu);
void CPX(struct cpu* cpu);
void CPY(struct cpu* cpu);
void DEC(struct cpu* cpu);
void DEX(struct cpu* cpu);
void DEY(struct cpu* cpu);
void EOR(struct cpu* cpu);
void INC(struct cpu* cpu);
void INX(struct cpu* cpu);
void INY(struct cpu* cpu);
void JMP(struct cpu* cpu);
void JSR(struct cpu* cpu);
void LDA(struct cpu* cpu);
void LDX(struct cpu* cpu);
void LDY(struct cpu* cpu);
void LSRA(struct cpu* cpu);
void LSRM(struct cpu* cpu);
void NOP(struct cpu* cpu);
void ORA(struct cpu* cpu);
void PHA(struct cpu* cpu);
void PHP(struct cpu* cpu);
void PLA(struct cpu* cpu);
void PLP(struct cpu* cpu);
void ROLA(struct cpu* cpu);
void ROLM(struct cpu* cpu);
void RORA(struct cpu* cpu);
void RORM(struct cpu* cpu);
void RTI(struct cpu* cpu);
void RTS(struct cpu* cpu);
void SBC(struct cpu* cpu);
void SEC(struct cpu* cpu);
void SED(struct cpu* cpu);
void SEI(struct cpu* cpu);
void STA(struct cpu* cpu);
void STX(struct cpu* cpu);
void STY(struct cpu* cpu);
void TAX(struct cpu* cpu);
void TAY(struct cpu* cpu);
void TSX(struct cpu* cpu);
void TXA(struct cpu* cpu);
void TXS(struct cpu* cpu);
void TYA(struct cpu* cpu);
/*illegal instructions*/
void ALR(struct cpu* cpu);
void ANC(struct cpu* cpu);
void ANE(struct cpu* cpu);
void ARR(struct cpu* cpu);
void DCP(struct cpu* cpu);
void ISC(struct cpu* cpu);
void LAS(struct cpu* cpu);
void LAX(struct cpu* cpu);
void LXA(struct cpu* cpu);
void RLA(struct cpu* cpu);
void RRA(struct cpu* cpu);
void SAX(struct cpu* cpu);
void SBX(struct cpu* cpu);
void SHA(struct cpu* cpu);
void SHX(struct cpu* cpu);
void SHY(struct cpu* cpu);
void SLO(struct cpu* cpu);
void SRE(struct cpu* cpu);
void TAS(struct cpu* cpu);
void JAM(struct cpu* cpu);

#include "nes.h"