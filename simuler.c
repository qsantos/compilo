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

#include "asm.h"
#include "u32stack.h"

#include <stdio.h>

void ASM_Simulate(ASM* a)
{
	u32* reg = (u32*) malloc(sizeof(u32) * a->reg);
	assert(reg);
	
	u32 ip = 0;

	u32stack* stack = NULL;;
	
	do
	{

#define r0 reg[a->code[ip].v.r.r0]
#define r1 reg[a->code[ip].v.r.r1]
#define r2 reg[a->code[ip].v.r.r2]

		u32 res = a->code[ip].v.r.r0;

		switch (a->code[ip++].insn)
		{
		case INSN_SET:
			u32 v = a->code[ip].v.r1;
			r0 = v;
			printf("r%d <- %d\n", r0, v);
			break;
		case INSN_MOV:  r0 = r1;          printf("r%d <- %d\n", res, r0); break;
		case INSN_NEG:  r0 = !r1;         printf("r%d <- %d\n", res, r0); break;
		case INSN_AND:  r0 = r1 & r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_OR:   r0 = r1 | r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_XOR:  r0 = r1 ^ r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_NOT:  r0 = ~r1;         printf("r%d <- %d\n", res, r0); break;
		case INSN_LAND: r0 = (r1 && r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_LOR:  r0 = (r1 || r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_EQ:   r0 = (r1 == r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_NEQ:  r0 = (r1 != r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_LE:   r0 = (r1 <= r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_LT:   r0 = (r1 <  r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_GE:   r0 = (r1 >= r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_GT:   r0 = (r1 >  r2);  printf("r%d <- %d\n", res, r0); break;
		case INSN_ADD:  r0 = r1 + r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_SUB:  r0 = r1 - r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_MUL:  r0 = r1 * r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_DIV:  r0 = r1 / r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_MOD:  r0 = r1 % r2;     printf("r%d <- %d\n", res, r0); break;
		case INSN_JMP:  ip = r0;          printf("goto %d\n",   ip);      break;
		case INSN_JZ:   if (r0)  ip = r1; printf("goto %d\n",   ip);      break;
		case INSN_JNZ:  if (!r0) ip = r1; printf("goto %d\n",   ip);      break;
		case INSN_CALL:
			u32 lbl = a->code[ip].v.p->head;
			RegList* l1 = a->code[lbl].v.p;
			RegList* l2 = a->code[ip].v.p->tail;
			while (l1)
			{
				reg[l1->head] = reg[l2->head];
				l1 = l1->tail;
				l2 = l2->tail;
			}
			u32stack_push(&stack, ip);
			ip = lbl;
			break;
		default:
			assert(false);
			break;
		}
	} while (getchar() != 'q');

	u32stack_delete(&stack);
	free(reg);
}
