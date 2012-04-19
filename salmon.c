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

void Salmon_BuildFlow(ASM* a, Context* c)
{
	u32 n = a->n_code;
	for (u32 i = 0; i < n; i ++)
	{
		a->code[i].s.jmp  = -1;
		a->code[i].s.use  = Set_New(n);
		a->code[i].s.out  = Set_New(n);
		a->code[i].s.def  = Set_New(n);
		
		switch (a->code[i].insn)
		{
		case INSN_SET:
			break;
		case INSN_MOV:  case INSN_NEG: case INSN_NOT:
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
		case INSN_LBL:
			/* TODO */
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
		for (s32 i = n-1; i >= 0; i --)
		{
			Set* tmp;
			
			tmp = Set_Diff(a->code[i].s.out, a->code[i].s.def);
			Set* in  = Set_Union(a->code[i].s.in, tmp);
			Set_Delete(tmp);
			
			changed = changed || Set_Cmp(in, a->code[i].s.in);
			Set_Delete(a->code[i].s.in);
			a->code[i].s.in = in;
			
			Set* out;
			if (i < (n-1))
			{
				if (a->code[i].s.jmp)
					out = Set_Union(a->code[i+1].s.in, a->code[a->code[i].s.jmp].s.in);
				else
					out = Set_Copy(a->code[i+1].s.in);
			}
			else
			{
				if (a->code[i].s.jmp)
					out = Set_Copy(a->code[a->code[i].s.jmp].s.in);
				else
					out = Set_New(n);
			}
			
			changed = changed || Set_Cmp(out, a->code[i].s.out);
			Set_Delete(a->code[i].s.out);
			a->code[i].s.out = out;
		}
	}
}
