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
	u32 n = a->n_code;
	u32stack* s;
	for (u32 i = 0; i < n; i ++)
	{
		a->code[i].s.jmp  = -1;
		a->code[i].s.use  = Set_New(n);
		a->code[i].s.out  = Set_New(n);
		a->code[i].s.def  = Set_New(n);
		
		switch (a->code[i].insn)
		{
		case INSN_SET:
			Set_Append(a->code[i].v.r.r0, a->code[i].s.def);
			break;
		case INSN_MOV:  case INSN_NOT: case INSN_LNOT:
			Set_Append(a->code[i].v.r.r0, a->code[i].s.def);
			Set_Append(a->code[i].v.r.r1, a->code[i].s.use);
			break;
		case INSN_AND:  case INSN_OR:  case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:   case INSN_NEQ: case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD:  case INSN_SUB: case INSN_MUL: case INSN_DIV:  case INSN_MOD:
			Set_Append(a->code[i].v.r.r0, a->code[i].s.def);
			Set_Append(a->code[i].v.r.r1, a->code[i].s.use);
			Set_Append(a->code[i].v.r.r2, a->code[i].s.use);
			break;
		case INSN_JMP:  case INSN_JZ:  case INSN_JNZ:
			a->code[i].s.jmp = a->labels[a->code[i].v.r.r0];
			break;
		case INSN_CALL:
			s = a->code[i].v.p->tail;
			while (s)
			{
				Set_Append(s->head, a->code[i].s.use);
				s = s->tail;
			}
			break;
		case INSN_RET:
			Set_Append(0, a->code[i].s.def);
			break;
		default:
			break;
		}
		
		a->code[i].s.in = Set_Copy(a->code[i].s.use);
	}
}

void Salmon_Vivacity(ASM* a)
{
	u32 n = a->n_code;	
	bool changed = true;
	while (changed)
	{
		changed = false;
		u32 i = n - 1;
		do
		{
			Set* tmp;
			tmp = Set_Diff(a->code[i].s.out, a->code[i].s.def);
			Set* in = Set_Union(a->code[i].s.use, tmp);
			Set_Delete(tmp);
			
			changed = changed || Set_Cmp(in, a->code[i].s.in);
			Set_Delete(a->code[i].s.in);
			a->code[i].s.in = in;
			
			Set* out;
			if (i < (n-1))
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
					out = Set_New(n);
			}
			
			changed = changed || Set_Cmp(out, a->code[i].s.out);
			Set_Delete(a->code[i].s.out);
			a->code[i].s.out = out;
			
			i--;
		} while (i);
	}
	
	//Salmon_VivacityDebug(n, a);
}

IntGraph* IntGraph_New(u32 n)
{
	IntGraph* g = (IntGraph*) malloc(sizeof(IntGraph));
	assert(g);
	
	g->n = n;
	g->d = (u32*) calloc(n, sizeof(u32));
	assert(g->d);
	g->e = (edge*) calloc(n * n, sizeof(edge));
	assert(g->e);
	
	return g;
}

void IntGraph_AddInterf(IntGraph* g, u32 i, u32 j)
{
	if (!g->e[i + j * g->n].interf)
	{
		g->e[i + j * g->n].interf = true;
		g->e[j + i * g->n].interf = true;
		g->d[i]++;
		g->d[j]++;
	}
}

void IntGraph_AddMove(IntGraph* g, u32 i, u32 j)
{
	if (!g->e[i + j * g->n].pref)
	{
		g->e[i + j * g->n].pref = true;
		g->e[j + i * g->n].pref = true;
		g->d[i]++;
		g->d[j]++;
	}
}

void IntGraph_Delete(IntGraph* g)
{
	free(g->d);
	free(g->e);
	free(g);
}

IntGraph* Salmon_Interference(ASM* a)
{
	IntGraph* g = IntGraph_New(a->n_regs);
	
	for (u32 i = 0; i < a->n_code; i++)
	{
		switch (a->code[i].insn)
		{
		case INSN_MOV:
			for (u32 j = 0; j < a->n_code; j++)
			{
				if (a->code[i].s.out->obj[j] || j != a->code[i].v.r.r1)
					IntGraph_AddMove(g, a->code[i].v.r.r0, j);
			}
			break;

		case INSN_SET:
		case INSN_NOT: case INSN_LNOT:
		case INSN_AND: case INSN_OR:  case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:  case INSN_NEQ: case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD: case INSN_SUB: case INSN_MUL: case INSN_DIV:  case INSN_MOD:
			for (u32 j = 0; j < a->n_code; j++)
			{
				if (a->code[i].s.out->obj[j])
					IntGraph_AddInterf(g, a->code[i].v.r.r0, j);
			}
			break;

		default:
			break;

		}
	}
	
	return g;
}

RegAlloc* Salmon_RegAlloc(IntGraph* g, u32 k)
{
}
