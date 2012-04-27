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

u32 preg(RegAlloc* ra, u32 vreg)
{
//	return 2 + vreg;
	assert(!ra[vreg].spilled);
	return 2 + ra[vreg].color;
}

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
		ASM_toMIPS_Pop(preg(ra, params->head));
	}
}

#define REG(VREG) preg(ra, i.v.r.VREG)
#define MIPS_BINOP(OP) printf("\t%s $%.2lu, $%.2lu, $%.2lu\n", OP, REG(r0), REG(r1), REG(r2));
void ASM_toMIPS(ASM* a, Context* c)
{
	assert(a);
	assert(c);
	
	u32       v;
	u32stack* args;
	u32stack* params;
	symbol    s;
	
	IntGraph* ig = Salmon_Interference(a);
	RegAlloc* ra = Salmon_RegAlloc(ig, N_REGS);
	
	printf("main:\n");
	
	for (u32 ip = 0; ip < a->n_code; ip++)
	{
		Instr i  = a->code[ip];
		
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
		case INSN_SET:  printf("\tli   $%.2lu,  %lu\n", REG(r0), i.v.r.r1); break;
		case INSN_MOV:  printf("\tmove $%.2lu, $%.2lu\n", REG(r0), REG(r1));  break;
		case INSN_NOT:  printf("\tnot  $%.2lu, $%.2lu\n", REG(r0), REG(r1));  break;
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
		case INSN_GT:   printf("\tslt $%.2lu, $%.2lu, $%.2lu\n", REG(r0), REG(r2), REG(r1)); break;
		case INSN_ADD:  MIPS_BINOP("add"); break;
		case INSN_SUB:  MIPS_BINOP("sub"); break;
		case INSN_MUL:  MIPS_BINOP("mul"); break;
		case INSN_DIV:
			printf("\tdiv  $%.2lu, $%.2lu\n", REG(r1), REG(r2));
			printf("\tmflo $%.2lu\n",  REG(r0));
			break;
		case INSN_MOD:
			printf("\tdiv  $%.2lu, $%.2lu\n", REG(r1), REG(r2));
			printf("\tmfhi $%.2lu\n",  REG(r0));
			break;
		case INSN_JMP:
			printf("\tj l%lu\n", i.v.r.r0);
			break;
		case INSN_JZ:
			printf("\tbeqz $%.2lu, l%lu\n", REG(r0), i.v.r.r1);
			break;
		case INSN_JNZ:
			printf("\tbnze $%.2lu, l%lu\n", REG(r0), i.v.r.r1);
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
				ASM_toMIPS_Push(preg(ra, params->head));
				params = params->tail;
			}
			
			// set params
			params = s.params;
			while (params && args)
			{
				printf("\tmove $%.2lu, $%.2lu\n", preg(ra, params->head), preg(ra, args->head));
				args   = args->tail;
				params = params->tail;
			}
			
			printf("\tjal l%lu\n", v);
			break;
		case INSN_RET:
			ASM_toMIPS_Pop(REG_RA);
			ASM_toMIPS_PopRegs(ra, c->st[i.v.r.r0].usedRegs);
			printf("\tjr $ra\n");
			break;
		case INSN_LBL:
			printf("l%lu:\n", i.v.r.r0);
			if (i.v.r.r1)
				ASM_toMIPS_Push(REG_RA);
			break;
		}
	}
}
