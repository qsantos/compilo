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
	u32stack* stackedRegs;
	u32stack* stackedRegCounts;
	symbol s;
	
	while (!stop && ip < a->n_code)
	{
		Instr i  = a->code[ip++];
		u32 res = i.v.r.r0;
		
		switch (i.insn)
		{
		case INSN_SET:
			v = i.v.r.r1;
			rr0 = v;
			printf("$%lu <- %lu\n", res, v);
			break;
		case INSN_MOV:  rr0 = rr1;           printf("$%lu <- $%lu (%lu)\n", res, i.v.r.r1, rr0); break;
		case INSN_NOT:  rr0 = ~rr1;          printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_AND:  rr0 = rr1 & rr2;     printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_OR:   rr0 = rr1 | rr2;     printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_XOR:  rr0 = rr1 ^ rr2;     printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_LNOT: rr0 = !rr1;          printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_LAND: rr0 = (rr1 && rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_LOR:  rr0 = (rr1 || rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_EQ:   rr0 = (rr1 == rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_NEQ:  rr0 = (rr1 != rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_LE:   rr0 = (rr1 <= rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_LT:   rr0 = (rr1 <  rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_GE:   rr0 = (rr1 >= rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_GT:   rr0 = (rr1 >  rr2);  printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_ADD:  rr0 = rr1 + rr2;     printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_SUB:  rr0 = rr1 - rr2;     printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_MUL:  rr0 = rr1 * rr2;     printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_DIV:  rr0 = rr1 / rr2;     printf("$%lu <- %lu\n", res, rr0); break;
		case INSN_MOD:  rr0 = rr1 % rr2;     printf("$%lu <- %lu\n", res, rr0); break;
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
			
			// non-params
			s = c->st[a->code[v].v.r.r1];
			params = s.usedRegs;
			for (v = 0; v < s.nonParamRegs; v++)
			{
				u32stack_push(&stack, regs[params->head]);
				u32stack_push(&stackedRegs, params->head);
				params = params->tail;
			}
			
			// params
			v = s.nonParamRegs;
			while (params && args)
			{
				u32stack_push(&stack, regs[params->head]);
				u32stack_push(&stackedRegs, params->head);
				v++;
				
				printf("# $%lu < $%lu (%lu)\n", params->head, args->head, regs[args->head]);
				regs[params->head] = regs[args->head];
				args   = args->tail;
				params = params->tail;
			}
			u32stack_push(&stackedRegCounts, v);
			break;
		case INSN_RET:
			v = u32stack_pop(&stackedRegCounts);
			while (v--)
				regs[u32stack_pop(&stackedRegs)] = u32stack_pop(&stack);
			ip = u32stack_pop(&stack);
			if (!stack)
				stop = true;
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
	assert(!ra[vreg].spilled);
	return ra[vreg].color;
}

#define REG(VREG) preg(ra, i.v.r.VREG)
#define MIPS_BINOP(OP) printf("\t%s $%lu, $%lu, $%lu", #OP, REG(r0), REG(r1), REG(r2));
void ASM_toMIPS(ASM* a, Context* c)
{
	assert(a);
	assert(c);
	
	IntGraph* ig = Salmon_Interference(a);
	RegAlloc* ra = Salmon_RegAlloc(ig, N_REGS);
	
//	u32 heap = 0;
//	for (u32 i = 0; i < N_REGS; i++)
//		if (ra[i].spilled)
//			ra[i].color = heap++;
	
	for (u32 ip = 0; ip < a->n_code; ip++)
	{
		Instr i  = a->code[ip++];
		
		switch (i.insn)
		{
		case INSN_SET:  printf("\tLI   $%lu,  %lu\n", REG(r0), i.v.r.r1); break;
		case INSN_MOV:  printf("\tMOVE $%lu, $%lu\n", REG(r0), REG(r1));  break;
		case INSN_NOT:  printf("\tNOT  $%lu, $%lu\n", REG(r0), REG(r1));  break;
		case INSN_AND:  MIPS_BINOP("AND"); break;
		case INSN_OR:   MIPS_BINOP("OR "); break;
		case INSN_XOR:  MIPS_BINOP("XOR"); break;
		case INSN_LNOT: break;
		case INSN_LAND: break;
		case INSN_LOR:  break;
		case INSN_EQ:   break;
		case INSN_NEQ:  break;
		case INSN_LE:   break;
		case INSN_LT:   MIPS_BINOP("SLT"); break;
		case INSN_GE:   break;
		case INSN_GT:   printf("\tSLT $%lu, $%lu, $%lu\n", REG(r0), REG(r2), REG(r1)); break;
		case INSN_ADD:  MIPS_BINOP("ADD"); break;
		case INSN_SUB:  MIPS_BINOP("SUB"); break;
		case INSN_MUL:  MIPS_BINOP("MUL"); break;
		case INSN_DIV:
			printf("\tMOVE $%lu, $%lu\n", REG(r0), REG(r1));
			printf("\tDIV  $%lu, $%lu\n", REG(r0), REG(r2));
			break;
		case INSN_MOD:
			printf("\tMOVE $%lu, $%lu\n",       REG_TMP0, REG(r1));
			printf("\tDIV  $%lu, $%lu\n",       REG_TMP0, REG(r2));
			printf("\tMUL  $%lu, $%lu, $%lu\n", REG_TMP0, REG_TMP0, REG(r2));
			printf("\tMOVE $%lu, $%lu\n",       REG(r0), REG(r1));
			printf("\tSUB  $%lu, $%lu, $%lu\n", REG(r0), REG(r0), REG_TMP0);
		case INSN_JMP:
			printf("\tJ l%lu\n", i.v.r.r0);
			break;
		case INSN_JZ:
			printf("\tBNEZ $%lu, 8\n", REG(r0));
			printf("\tJ    l%lu\n", i.v.r.r1);
			break;
		case INSN_JNZ:
			printf("\tBEQZ $%lu, 8\n", REG(r0));
			printf("\tJ    l%lu\n", i.v.r.r1);
			break;
		case INSN_CALL: break;
		case INSN_RET:  break;
		case INSN_LBL:  printf("l%lu:\n", i.v.r.r0); break;
		}
	}
}
