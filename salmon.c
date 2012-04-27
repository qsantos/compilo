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

#include "salmon.h"

#include <stdlib.h>
#include <assert.h>

/* XXX: DEBUG */
/*void Set_Display(Set* s)
{
	for (u32 i = 0; i < s->n; i++)
		if (s->obj[i])
			printf("%lu, ", i);
	printf("\n");
}

void Salmon_VivacityDebug(u32 n, ASM* a)
{
	for (u32 i = 0; i < n; i ++)
	{
		printf("-- %lu\n", i);
		printf("in: ");
		Set_Display(a->code[i].s.in);
		printf("out: ");
		Set_Display(a->code[i].s.out);
		printf("def: ");
		Set_Display(a->code[i].s.def);
		printf("use: ");
		Set_Display(a->code[i].s.use);
	}
}*/

void Salmon_BuildFlow(ASM* a)
{
	u32stack* s;
	for (u32 i = 0; i < a->n_code; i ++)
	{
		a->code[i].s.jmp = -1;
		a->code[i].s.use = Set_New(a->n_regs);
		a->code[i].s.out = Set_New(a->n_regs);
		a->code[i].s.def = Set_New(a->n_regs);
		
		switch (a->code[i].insn)
		{
		case INSN_SET:
			Set_Append(a->code[i].s.def, a->code[i].v.r.r0);
			break;
		case INSN_MOV:  case INSN_NOT: case INSN_LNOT:
			Set_Append(a->code[i].s.def, a->code[i].v.r.r0);
			Set_Append(a->code[i].s.use, a->code[i].v.r.r1);
			break;
		case INSN_AND:  case INSN_OR:  case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:   case INSN_NEQ: case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD:  case INSN_SUB: case INSN_MUL: case INSN_DIV:  case INSN_MOD:
			Set_Append(a->code[i].s.def, a->code[i].v.r.r0);
			Set_Append(a->code[i].s.use, a->code[i].v.r.r1);
			Set_Append(a->code[i].s.use, a->code[i].v.r.r2);
			break;
		case INSN_JMP:  case INSN_JZ:  case INSN_JNZ:
			a->code[i].s.jmp = a->labels[a->code[i].v.r.r0];
			break;
		case INSN_CALL:
			s = a->code[i].v.p->tail;
			while (s)
			{
				Set_Append(a->code[i].s.use, s->head);
				s = s->tail;
			}
			break;
		case INSN_RET:
			Set_Append(a->code[i].s.def, 0);
			break;
		case INSN_STOP:
		case INSN_LBL:
			break;
		}
		
		a->code[i].s.in = Set_Copy(a->code[i].s.use);
	}
}

void Salmon_Vivacity(ASM* a)
{
	bool changed = true;
	while (changed)
	{
		changed = false;
		u32 i = a->n_code;
		while (i)
		{
			i--;
			
			Set* tmp;
			tmp = Set_Diff(a->code[i].s.out, a->code[i].s.def);
			Set* in = Set_Union(a->code[i].s.use, tmp);
			Set_Delete(tmp);
			
			if (Set_Cmp(in, a->code[i].s.in))
				changed = true;
			Set_Delete(a->code[i].s.in);
			a->code[i].s.in = in;
			
			Set* out;
			if (i < a->n_code - 1)
			{
				if (a->code[i].s.jmp != -1)
					out = Set_Union(a->code[i+1].s.in, a->code[a->code[i].s.jmp].s.in);
				else
					out = Set_Copy(a->code[i+1].s.in);
			}
			else
			{
				if (a->code[i].s.jmp != -1)
					out = Set_Copy(a->code[a->code[i].s.jmp].s.in);
				else
					out = Set_New(a->n_regs);
			}
			
			if (Set_Cmp(out, a->code[i].s.out))
				changed = true;
			Set_Delete(a->code[i].s.out);
			a->code[i].s.out = out;
		}
	}
	
	//Salmon_VivacityDebug(n, a);
}

