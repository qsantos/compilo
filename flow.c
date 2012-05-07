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

#include "flow.h"

#include <stdlib.h>
#include <assert.h>

void Flow_Print(Flow* f)
{
	for (u32 i = 0; i < f->ns; i++)
	{
		printf("-- %lu\n", i);
		printf("in: " ); Set_Print(f->s[i].in );      printf("\n");
		printf("out: "); Set_Print(f->s[i].out);      printf("\n");
		printf("def: "); u32stack_Print(f->s[i].def); printf("\n");
		printf("use: "); u32stack_Print(f->s[i].use); printf("\n");
	}
}

Flow* Flow_New(u32 ns, u32 nr)
{
	Flow* f = (Flow*) malloc(sizeof(Flow));
	assert(f);
	f->ns = ns;
	f->nr = nr;
	f->s = (Salmon*) malloc(ns * sizeof(Salmon));
	assert(f->s);
	for (u32 i = 0; i < ns; i++)
	{
		f->s[i].def = NULL;
		f->s[i].use = NULL;
		
		f->s[i].in = Set_New(nr);
		f->s[i].out = Set_New(nr);
		
		f->s[i].mov = -1;
		f->s[i].jmp = -1;
	}
	
	return f;
}

void Flow_Delete(Flow* f)
{
	assert(f);
	
	for (u32 i = 0; i < f->ns; i++)
	{
		u32stack_Delete(&f->s[i].def);
		u32stack_Delete(&f->s[i].use);
		Set_Delete(f->s[i].in);
		Set_Delete(f->s[i].out);
	}
	free(f->s);
	free(f);
}

Flow* Flow_Build(ASM* a, u32 s, u32 e, Context* c)
{
	assert(a);
	
	Flow* f = Flow_New(e - s + 1, a->n_regs);
	
	u32 k = 0;
	for (u32 i = s; i <= e; i++)
	{
		Instr ins = a->code[i];
		switch (ins.insn)
		{
		case INSN_AND:  case INSN_OR:  case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:   case INSN_NEQ: case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD:  case INSN_SUB: case INSN_MUL: case INSN_DIV:  case INSN_MOD:
			u32stack_Push(&f->s[k].def, ins.v.r.r0);
			u32stack_Push(&f->s[k].use, ins.v.r.r1);
			u32stack_Push(&f->s[k].use, ins.v.r.r2);
			break;
		case INSN_NOT: case INSN_LNOT:
			u32stack_Push(&f->s[k].def, ins.v.r.r0);
			u32stack_Push(&f->s[k].use, ins.v.r.r1);
			break;
		case INSN_SET:
			u32stack_Push(&f->s[k].def, ins.v.r.r0);
			break;
		case INSN_MOV:
			u32stack_Push(&f->s[k].def, ins.v.r.r0);
			u32stack_Push(&f->s[k].use, ins.v.r.r1);
			f->s[k].mov = ins.v.r.r1;
			break;
		case INSN_JMP:  case INSN_JZ:  case INSN_JNZ:
			f->s[k].jmp = a->labels[ins.v.r.r0];
			assert((s32)s <= f->s[k].jmp);
			assert(f->s[k].jmp <= (s32)e);
			f->s[k].jmp -= s;
			break;
		case INSN_CALL:
			f->s[k].use = u32stack_RCopy(ins.v.p->tail->tail);
			u32stack_Push(&f->s[k].def, ins.v.p->tail->head);
			break;
		case INSN_LBL:
			if (ins.v.r.r1 == 1)
				f->s[k].def = u32stack_RCopy(c->st[ins.v.r.r2].params);
			break;
		case INSN_RET:
			u32stack_Push(&f->s[k].use, ins.v.r.r1);
			break;
		case INSN_STOP:
			break;
		}
		
		k++;
	}
	
	return f;
}

void Flow_Vivacity(Flow* f)
{
	assert(f);
	
	bool changed = true;
	while (changed)
	{
		changed = false;
		s32 e = f->ns-1;
		for (s32 k = e; k >= 0; k--)
		{
			Set* new_in = Set_UnionDiff(f->s[k].use, f->s[k].out, f->s[k].def);
			
			Set* next = (k == e          ? NULL : f->s[k + 1].in);
			Set* jmp  = (f->s[k].jmp < 0 ? NULL : f->s[f->s[k].jmp].in);
			Set* new_out  = Set_UnionNull(next, jmp, f->nr);
			
			if (Set_Cmp(new_out, f->s[k].out) || Set_Cmp(new_in, f->s[k].in))
				changed = true;
			
			Set_Delete(f->s[k].in);
			Set_Delete(f->s[k].out);
			f->s[k].in  = new_in;
			f->s[k].out = new_out;
		}
	}
}
