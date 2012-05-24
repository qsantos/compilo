/*\
 *       \\          A Tiny C to MIPS Compiler                     //
 *        \\_        Copyright (C) 2012 Thomas  GREGOIRE         _//
 *     .---(')                          Quentin SANTOS          (')---. 
 *   o( )_-\_        Logos by jgs                                _/-_( )o
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

#include "mips.h"

#include <assert.h>
#include <stdlib.h>

#include "flow.h"
#include "regalloc.h"

#define REG_TMP0 ((u32)24)
#define REG_TMP1 ((u32)25)
#define REG_RA   31

u32 ra_wreg(u32 treg, RegAlloc ra)
{
	return ra.spilled ? treg : 8 + ra.color;
}
void ra_sreg(u32 treg, RegAlloc ra)
{
	if (ra.spilled)
	{
		printf("\taddi $sp, $sp, -%lu\t#SAVE S%lu\n", 4 * ra.color, ra.color);
		printf("\tsw   $%.2lu, 0($sp)\n", treg);
		printf("\taddi $sp, $sp,  %lu\n", 4 * ra.color);
	}
}
u32 ra_rreg(u32 treg, RegAlloc ra)
{
	if (ra.spilled)
	{
		printf("\taddi $sp, $sp, -%lu\t#LOAD S%lu\n", 4 * ra.color, ra.color);
		printf("\tlw   $%.2lu, 0($sp)\n", treg);
		printf("\taddi $sp, $sp,  %lu\n", 4 * ra.color);
		return treg;
	}
	else
	{
		return 8 + ra.color;
	}
}
#define WREG_(VREG)  ra_wreg(REG_TMP0, ra[VREG])
#define SREG_(VREG)  ra_sreg(REG_TMP0, ra[VREG])
#define RREG1_(VREG) ra_rreg(REG_TMP0, ra[VREG])
#define RREG2_(VREG) ra_rreg(REG_TMP1, ra[VREG])
#define WREG(IREG)   WREG_(i.v.r.IREG)
#define SREG(IREG)   SREG_(i.v.r.IREG)
#define RREG1(IREG)  RREG1_(i.v.r.IREG)
#define RREG2(IREG)  RREG2_(i.v.r.IREG)

static void DecSP(u32 v)
{
	if (v)
		printf("\taddi $sp, $sp, -%lu\n", v);
}
static void IncSP(u32 v)
{
	if (v)
		printf("\taddi $sp, $sp, %lu\n", v);
}
static void Push(u32 reg)
{
	printf("\taddi $sp, $sp, -4\t#PUSH $%.2lu\n", reg);
	printf("\tsw   $%.2lu, 0($sp)\n", reg);
}
static void Pop(u32 reg)
{
	printf("\tlw   $%.2lu, 0($sp)\t#POP $%.2lu\n", reg, reg);
	printf("\taddi $sp, $sp, 4\n");
}
static void PopRegs(RegAlloc* ra, u32stack* params)
{
	if (params)
	{
		PopRegs(ra, params->tail);
		if (!ra[params->head].spilled)
			Pop(WREG_(params->head));
	}
}
static void GetSpilledParams(RegAlloc* ra, u32stack* params)
{
	if (params)
	{
		GetSpilledParams(ra, params->tail);
		printf("\tlw   $%.2lu, 0($%lu)\n", WREG_(params->head), REG_TMP1);
		printf("\taddi $%.2lu, $%.2lu, 4\n", REG_TMP1, REG_TMP1);
		SREG_(params->head);
	}
}

#define MIPS_BINOP(OP)                                                             \
	printf("\t%s $%.2lu, $%.2lu, $%.2lu\n", OP, WREG(r0), RREG1(r1), RREG2(r2)); \
	SREG(r0);                                                                  \

void OutputFile(const char* name)
{
	char buffer[1024];
	FILE* f = fopen(name, "r");
	while (!feof(f))
	{
		u32 n = fread(buffer, 1, 1024, f);
		fwrite(buffer, 1, n, stdout);
	}
	fclose(f);
}

void ASM_toMIPS(ASM* a, Context* c)
{
	assert(a);
	assert(c);
	
	u32 k = 16;
	
	u32       v;
	u32       w;
	u32stack* args;
	u32stack* params;
	symbol    symb;
	u32       depth = 0;
	
	printf("# memory allocation misc\n");
	OutputFile("malloc/mem.s");
	
	printf("#malloc\nl%lu:\n", Context_Get(c, "malloc")->label);
	OutputFile("malloc/malloc.s");
	
	printf("#free\nl%lu:\n", Context_Get(c, "free")->label);
	OutputFile("malloc/free.s");
	
	printf("main:\n");
	
	u32 s = 0;
	u32 e;
	while (s < a->n_code)
	{
		e = s + 1;
		while (e < a->n_code)
		{
			if (a->code[e].insn == INSN_LBL && a->code[e].v.r.r1 == 1)
				break;
			e++;
		}
		e--;
		
		Flow* f = Flow_Build(a, s, e, c);
		Flow_Vivacity(f);
		IntGraph* g = IntGraph_FromFlow(f);
		RegAlloc* ra = IntGraph_RegAlloc(g, k, a->spilled);
		IntGraph_Delete(g);
		Flow_Delete(f);
		
		for (u32 ip = s; ip <= e; ip++)
		{
			Instr i = a->code[ip];
			
			switch (i.insn)
			{
			case INSN_STOP:
				Push(2);
				printf("\tli $v0, 4\n");
				printf("\tla, $a0, endMsgA\n");
				printf("\tsyscall\n");
				
				printf("\tli $v0, 1\n");
				Pop(4);
				printf("\tsyscall\n");
				
				printf("\tli $v0, 4\n");
				printf("\tla, $a0, endMsgB\n");
				printf("\tsyscall\n");
				
				printf("\tli $v0, 10\n");
				printf("\tsyscall\n");
				
				printf("\t.data\n");
				printf("\tendMsgA: .asciiz \"The program returned \"\n");
				printf("\tendMsgB: .asciiz \".\\n\"\n");
				printf("\t.text\n");
				break;
			case INSN_SET:  printf("\tli   $%.2lu,  %lu\n",   WREG(r0), i.v.r.r1);  SREG(r0); break;
			case INSN_MOV:  printf("\tmove $%.2lu, $%.2lu\n", WREG(r0), RREG1(r1)); SREG(r0); break;
			case INSN_MRD:  printf("\tlw   $%.2lu, 0($%.2lu)\n", RREG1(r0), RREG2(r1)); break;
			case INSN_MWR:  printf("\tsw   $%.2lu, 0($%.2lu)\n", RREG1(r1), RREG2(r0)); break;
			case INSN_RGA:
				assert(ra[i.v.r.r1].spilled);
				printf("\taddiu $%.2lu, $sp, %lu\n", WREG(r0), 4*ra[i.v.r.r1].color);
				break;
			case INSN_NOT:  printf("\tnot  $%.2lu, $%.2lu\n", WREG(r0), RREG1(r1)); SREG(r0); break;
			case INSN_AND:  MIPS_BINOP("and"); break;
			case INSN_OR:   MIPS_BINOP("or "); break;
			case INSN_XOR:  MIPS_BINOP("xor"); break;
			case INSN_LNOT: printf("\tnot  $%.2lu, $%.2lu\n", WREG(r0), RREG1(r1)); SREG(r0); break;break;
			case INSN_LAND: MIPS_BINOP("and");
			case INSN_LOR:  MIPS_BINOP("or ");
			case INSN_EQ:   break; // TODO
			case INSN_NEQ:  break; // TODO
			case INSN_LE:   break; // TODO
			case INSN_LT:   MIPS_BINOP("slt"); break;
			case INSN_GE:   break; // TODO
			case INSN_GT:   printf("\tslt $%.2lu, $%.2lu, $%.2lu\n", WREG(r0), RREG1(r2), RREG2(r1)); SREG(r0); break;
			case INSN_ADD:  MIPS_BINOP("add"); break;
			case INSN_SUB:  MIPS_BINOP("sub"); break;
			case INSN_MUL:  MIPS_BINOP("mul"); break;
			case INSN_DIV:
				printf("\tdiv  $%.2lu, $%.2lu\n", RREG1(r1), RREG2(r2));
				printf("\tmflo $%.2lu\n", WREG(r0));
				SREG(r0);
				break;
			case INSN_MOD:
				printf("\tdiv  $%.2lu, $%.2lu\n", RREG1(r1), RREG2(r2));
				printf("\tmfhi $%.2lu\n", WREG(r0));
				SREG(r0);
				break;
			case INSN_JMP:
				printf("\tj l%lu\n", i.v.r.r0);
				break;
			case INSN_JZ:
				printf("\tbeqz $%.2lu, l%lu\n", RREG1(r1), i.v.r.r0);
				break;
			case INSN_JNZ:
				printf("\tbnze $%.2lu, l%lu\n", RREG1(r1), i.v.r.r0);
				break;
			case INSN_CALL:
				args = i.v.p;
				v = args->head;
				args = args->tail;
				
				w = args->head;
				args = args->tail;
				
				// set args
				if (args){printf("\tmove $a0, $%.2lu\n", RREG1_(args->head)); args=args->tail;}
				if (args){printf("\tmove $a1, $%.2lu\n", RREG1_(args->head)); args=args->tail;}
				if (args){printf("\tmove $a2, $%.2lu\n", RREG1_(args->head)); args=args->tail;}
				if (args){printf("\tmove $a3, $%.2lu\n", RREG1_(args->head)); args=args->tail;}
				while (args)
				{
					Push(RREG1_(args->head));
					args = args->tail;
				}
				
				printf("\tjal l%lu\n", v);
				printf("\tmove $%.2lu, $v0\n", RREG1_(w));
				
				// pop args
				args = i.v.p;
				args = args->tail;
				args = args->tail;
				if (args) args = args->tail;
				if (args) args = args->tail;
				if (args) args = args->tail;
				if (args) args = args->tail;
				v = 0;
				while (args)
				{
					v +=4;
					args = args->tail;
				}
				IncSP(v);
				break;
			case INSN_RET:
				printf("\tmove $v0, $%.2lu\n", RREG1(r1));
				IncSP(4 * depth);
				PopRegs(ra, c->st[i.v.r.r0].usedRegs);
				Pop(REG_RA);
				printf("\tjr $ra\n");
				break;
			case INSN_LBL:
				printf("l%lu:\n", i.v.r.r0);
				if (i.v.r.r1)
				{
					symb = c->st[i.v.r.r2];
					
					// save spilled parameters stack position
					printf("\tmove $%.2lu, $sp\n", REG_TMP1);
					
					// save regs
					Push(REG_RA);
					params = symb.usedRegs;
					while (params)
					{
						if (!ra[params->head].spilled)
							Push(RREG1_(params->head));
						params = params->tail;
					}
					
					// prepare the stack frame
					depth = 0;
					params = symb.usedRegs;
					while (params)
					{
						if (ra[params->head].spilled)
							ra[params->head].color = depth++;
						params = params->tail;
					}
					DecSP(4 * depth);
					
					// ingoing parameters
					params = symb.params;
					v = 0;
					while (params && v < 4)
					{
						printf("\tmove $%.2lu, $a%lu\n", WREG_(params->head), v);
						SREG_(params->head);
						params = params->tail;
						v++;
					}
					GetSpilledParams(ra, params);
				}
				break;				
			}
		}
		
		s = e + 1;
	}
	
}
