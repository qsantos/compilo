/*\
 *       \\          A Tiny C to SPIM Compiler                     //
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

#include "exec.h"

#include <assert.h>
#include <stdlib.h>

#include "salmon.h"
#include "regalloc.h"

void ASM_Simulate_PopRegs(u32* regs, u32stack** stack, u32stack* params)
{
	if (params)
	{
		ASM_Simulate_PopRegs(regs, stack, params->tail);
		regs[params->head] = u32stack_pop(stack);
	}
}

#define rr0 regs[i.v.r.r0]
#define rr1 regs[i.v.r.r1]
#define rr2 regs[i.v.r.r2]
void ASM_Simulate(ASM* a, Context* c)
{
	assert(a);
	
	u32* regs = (u32*) malloc(sizeof(u32) * a->n_regs);
	assert(regs);
	
	bool stop = false;
	u32 ip = 0;
	u32 v;
	u32stack* stack = NULL;
	u32stack* args;
	u32stack* params;
	symbol s;
	
	while (!stop && ip < a->n_code)
	{
		Instr i  = a->code[ip++];
		u32 res = i.v.r.r0;
		
		switch (i.insn)
		{
		case INSN_STOP:
			stop = true;
			break;
		case INSN_SET:
			v = i.v.r.r1;
			rr0 = v;
			printf("$%.2lu <- %lu\n", res, v);
			break;
		case INSN_MOV:  rr0 = rr1;           printf("$%.2lu <- $%.2lu (%lu)\n", res, i.v.r.r1, rr0); break;
		case INSN_NOT:  rr0 = ~rr1;          printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_AND:  rr0 = rr1 & rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_OR:   rr0 = rr1 | rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_XOR:  rr0 = rr1 ^ rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_LNOT: rr0 = !rr1;          printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_LAND: rr0 = (rr1 && rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_LOR:  rr0 = (rr1 || rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_EQ:   rr0 = (rr1 == rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_NEQ:  rr0 = (rr1 != rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_LE:   rr0 = (rr1 <= rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_LT:   rr0 = (rr1 <  rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_GE:   rr0 = (rr1 >= rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_GT:   rr0 = (rr1 >  rr2);  printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_ADD:  rr0 = rr1 + rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_SUB:  rr0 = rr1 - rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_MUL:  rr0 = rr1 * rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_DIV:  rr0 = rr1 / rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_MOD:  rr0 = rr1 % rr2;     printf("$%.2lu <- %lu\n", res, rr0); break;
		case INSN_JMP:
			ip = a->labels[i.v.r.r0];
			printf("goto .%lu\n", i.v.r.r0);
			break;
		case INSN_JZ:
			if (!rr0)
			{
				ip = a->labels[i.v.r.r1];
				printf("goto .%lu\n", i.v.r.r1);
			}
			break;
		case INSN_JNZ:
			if (rr0)
			{
				ip = a->labels[rr1];
				printf("goto .%lu\n", i.v.r.r1);
			}
			break;
		case INSN_CALL:
			u32stack_push(&stack, ip);
			
			args = i.v.p;
			printf("Call .%lu\n", args->head);
			v = a->labels[args->head];
			args = args->tail;
			ip = v;
			
			// save regs
			s = c->st[a->code[v].v.r.r2];
			params = s.usedRegs;
			while (params)
			{
				u32stack_push(&stack, regs[params->head]);
				params = params->tail;
			}
			
			// set params
			params = s.params;
			while (params && args)
			{
				printf("# $%.2lu < $%.2lu (%lu)\n", params->head, args->head, regs[args->head]);
				regs[params->head] = regs[args->head];
				args   = args->tail;
				params = params->tail;
			}
			break;
		case INSN_RET:
			ASM_Simulate_PopRegs(regs, &stack, c->st[i.v.r.r0].usedRegs);
			ip = u32stack_pop(&stack);
			break;
		case INSN_LBL:
			break;
		}
	}

	u32stack_delete(&stack);
	free(regs);
}

u32 ra_wreg(u32 treg, RegAlloc ra)
{
	return ra.spilled ? treg : 2 + ra.color;
}
void ra_sreg(u32 treg, RegAlloc ra)
{
	if (ra.spilled)
	{
		printf("\taddi $sp, $sp, -%lu\n", 4 * ra.color);
		printf("\tsw   $%.2lu, 0($sp)", treg);
		printf("\taddi $sp, $sp   %lu\n", 4 * ra.color);
	}
}
u32 ra_rreg(u32 treg, RegAlloc ra)
{
	if (ra.spilled)
	{
		printf("\taddi $sp, $sp, -%lu\n", 4 * ra.color);
		printf("\tlw   $%.2lu, 0($sp)", treg);
		printf("\taddi $sp, $sp   %lu\n", 4 * ra.color);
		return treg;
	}
	else
	{
		return 2 + ra.color;
	}
}
#define REG0_(VREG) ra_wreg(24, ra[VREG])
#define REGS_(VREG) ra_sreg(24, ra[VREG])
#define REG1_(VREG) ra_rreg(24, ra[VREG])
#define REG2_(VREG) ra_rreg(25, ra[VREG])
#define REG0(IREG)  REG0_(i.v.r.IREG)
#define REGS(IREG)  REGS_(i.v.r.IREG)
#define REG1(IREG)  REG1_(i.v.r.IREG)
#define REG2(IREG)  REG2_(i.v.r.IREG)

void ASM_toMIPS_Push(u32 reg)
{
	printf("\taddi $sp, $sp, -4\t#PUSH $%.2lu\n", reg);
	printf("\tsw   $%.2lu, 0($sp)\n", reg);
}
void ASM_toMIPS_Pop(u32 reg)
{
	printf("\tlw   $%.2lu, 0($sp)\t#POP $%.2lu\n", reg, reg);
	printf("\taddi $sp, $sp, 4\n");
}
void ASM_toMIPS_PopRegs(RegAlloc* ra, u32stack* params)
{
	if (params)
	{
		ASM_toMIPS_PopRegs(ra, params->tail);
		if (!ra[params->head].spilled)
			ASM_toMIPS_Pop(REG0_(params->head));
	}
}
void ASM_toMIPS_PopParams(RegAlloc* ra, u32stack* params)
{
	if (params)
	{
		ASM_toMIPS_PopParams(ra, params->tail);
		ASM_toMIPS_Pop(REG0_(params->head));
		REGS_(params->head);
	}
}

#define MIPS_BINOP(OP)                                                             \
	printf("\t%s $%.2lu, $%.2lu, $%.2lu\n", OP, REG0(r0), REG1(r1), REG2(r2)); \
	REGS(r0);                                                                  \

void ASM_toMIPS(ASM* a, Context* c)
{
	assert(a);
	assert(c);
	
	u32       v;
	u32stack* args;
	u32stack* params;
	symbol    s;
	u32       depth = 0;

	printf("main:\n");
	
	u32 _s = 0;
	u32 e;
	while (_s < a->n_code)
	{
		e = ++_s;
		while (e < a->n_code)
		{
			if (a->code[e].insn == INSN_LBL && a->code[e].v.r.r1 == 1)
				break;
			e++;
		}

		Salmon_BuildFlow(a, _s, e-1);
		Salmon_Vivacity(a,  _s, e-1);
		IntGraph* ig = Salmon_Interference(a, _s, e-1);
		RegAlloc* ra = Salmon_RegAlloc(ig, N_REGS);
		
		for (u32 ip = _s; ip < e; ip++)
		{
			Instr i = a->code[ip];
			
			switch (i.insn)
			{
			case INSN_STOP:
				ASM_toMIPS_Push(2);
				printf("\tli $v0, 4\n");
				printf("\tla, $a0, endMsgA\n");
				printf("\tsyscall\n");
				
				printf("\tli $v0, 1\n");
				ASM_toMIPS_Pop(4);
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
			case INSN_SET:  printf("\tli   $%.2lu,  %lu\n",   REG0(r0), i.v.r.r1); REGS(r0); break;
			case INSN_MOV:  printf("\tmove $%.2lu, $%.2lu\n", REG0(r0), REG1(r1)); REGS(r0); break;
			case INSN_NOT:  printf("\tnot  $%.2lu, $%.2lu\n", REG0(r0), REG1(r1)); REGS(r0); break;
			case INSN_AND:  MIPS_BINOP("and"); break;
			case INSN_OR:   MIPS_BINOP("or "); break;
			case INSN_XOR:  MIPS_BINOP("xor"); break;
			case INSN_LNOT: break;
			case INSN_LAND: break;
			case INSN_LOR:  break;
			case INSN_EQ:   break;
			case INSN_NEQ:  break;
			case INSN_LE:   break;
			case INSN_LT:   MIPS_BINOP("slt"); break;
			case INSN_GE:   break;
			case INSN_GT:   printf("\tslt $%.2lu, $%.2lu, $%.2lu\n", REG0(r0), REG1(r2), REG2(r1)); REGS(r0); break;
			case INSN_ADD:  MIPS_BINOP("add"); break;
			case INSN_SUB:  MIPS_BINOP("sub"); break;
			case INSN_MUL:  MIPS_BINOP("mul"); break;
			case INSN_DIV:
				printf("\tdiv  $%.2lu, $%.2lu\n", REG1(r1), REG2(r2));
				printf("\tmflo $%.2lu\n", REG0(r0));
				REGS(r0);
				break;
			case INSN_MOD:
				printf("\tdiv  $%.2lu, $%.2lu\n", REG1(r1), REG2(r2));
				printf("\tmfhi $%.2lu\n", REG0(r0));
				REGS(r0);
				break;
			case INSN_JMP:
				printf("\tj l%lu\n", i.v.r.r0);
				break;
			case INSN_JZ:
				printf("\tbeqz $%.2lu, l%lu\n", REG1(r0), i.v.r.r1);
				break;
			case INSN_JNZ:
				printf("\tbnze $%.2lu, l%lu\n", REG1(r0), i.v.r.r1);
				break;
			case INSN_CALL:
				args = i.v.p;
				v = args->head;
				args = args->tail;
				
				// save regs
				s = c->st[a->code[a->labels[v]].v.r.r2];
				params = s.usedRegs;
				while (params)
				{
					if (!ra[params->head].spilled)
						ASM_toMIPS_Push(REG0_(params->head));
					params = params->tail;
				}
				
				// set args
				if (args){printf("\tmove $a0, $%.2lu\n", REG1_(params->head)); params = params->tail;}
				if (args){printf("\tmove $a1, $%.2lu\n", REG1_(params->head)); params = params->tail;}
				if (args){printf("\tmove $a2, $%.2lu\n", REG1_(params->head)); params = params->tail;}
				if (args){printf("\tmove $a3, $%.2lu\n", REG1_(params->head)); params = params->tail;}
				while (args)
				{
					ASM_toMIPS_Push(REG1_(args->head));
					args = args->tail;
				}
				
				printf("\tjal l%lu\n", v);
				break;
			case INSN_RET:
				printf("\taddi $sp, $sp, %lu\n", 4 * depth);
				ASM_toMIPS_Pop(REG_RA);
				ASM_toMIPS_PopRegs(ra, c->st[i.v.r.r0].usedRegs);
				printf("\tjr $ra\n");
				break;
			case INSN_LBL:
				printf("l%lu:\n", i.v.r.r0);
				if (i.v.r.r1)
				{
					s = c->st[i.v.r.r2];
					
					// ingoing parameters
					params = s.params;
					if(params){printf("\tmove $%.2lu $a0\n",REG0_(params->head));params=params->tail;}
					if(params){printf("\tmove $%.2lu $a1\n",REG0_(params->head));params=params->tail;}
					if(params){printf("\tmove $%.2lu $a2\n",REG0_(params->head));params=params->tail;}
					if(params){printf("\tmove $%.2lu $a3\n",REG0_(params->head));params=params->tail;}
					ASM_toMIPS_PopParams(ra, params);
					
					ASM_toMIPS_Push(REG_RA);
					
					// prepare the stack frame
					depth = 0;
					params = s.usedRegs;
					while (params)
					{
						if (ra[params->head].spilled)
							ra[params->head].color = depth++;
						params = params->tail;
					}
					printf("\taddi $sp, $sp, -%lu\n", 4 * depth);
				}
				break;				
			}
		}
		
		_s = e;
	}
	
}
