/*\
 *       \\          A Tiny C to SPIM Compiler                     //
 *        \\_        Copyright (C) 2012 Thomas  REGOIRE          _//
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

#include <assert.h>
#include <stdlib.h>

ASM* ASM_New()
{
	ASM* ret   = (ASM*) malloc(sizeof(ASM));
	assert(ret);
	ret->len   = 0;
	ret->avail = 1024;
	ret->code  = (Instr*) malloc(sizeof(Instr) * ret->avail);
	assert(ret->code);
	ret->reg   = 0;
	return ret;
}

void ASM_Delete(ASM* a)
{
	assert(a);
	free(a->code);
	free(a);
}

void ASM_Push(ASM* a, ASM_INSN insn, u32 r0, u32 r1, u32 r2)
{
	assert(a);
	if (a->len == a->avail)
	{
		a->avail *= 2;
		a->code = (Instr*) realloc(a->code, sizeof(Instr) * a->avail);
		assert(a->code);
	}
	Instr i = { insn, { { r0, r1, r2 } } };
	a->code[a->len++] = i;
}

u32 ASM_NewReg(ASM* a)
{
	return a->reg++;
}

#define ASM_UNIOP(INSTR) \
	r0 = ASM_GenExpr(a, c, e->v.uni_op); \
	ASM_Push(a, INSTR, r0, r0, 0); \
	break;
#define ASM_BINOP(INSTR) \
	r0 = ASM_GenExpr(a, c, e->v.bin_op.left); \
	r1 = ASM_GenExpr(a, c, e->v.bin_op.right); \
	ASM_Push(a, INSTR, r0, r0, r1); \
	break;

u32 ASM_GenExpr(ASM* a, Context* c, Expr* e)
{
	assert(a);
	assert(c);
	assert(e);
	
	u32 r0 = 0;
	u32 r1;
	u32 r2;
	
	switch (e->type)
	{
	case EXPR_INTEGER:
		r0 = ASM_NewReg(a);
		ASM_Push(a, INSN_SET, r0, e->v.i, 0);
		break;
	case EXPR_FUN_CALL: // TODO
		r0 = ASM_NewReg(a);
		c->st[e->v.call.id].reg = r0;
		break;
	case EXPR_AFF:
		r0 = ASM_NewReg(a);
		c->st[e->v.aff.id].reg = r0;
		if (e->v.aff.expr)
		{
			r1 = ASM_GenExpr(a, e->v.aff.expr, c);
			ASM_Push(a, INSN_MOV, r0, r1, 0);
		}
		break;
	case EXPR_VAR:
		r0 = c->st[e->v.aff.id].reg;
		break;
	
	case EXPR_NEG:  ASM_UNIOP(INSN_NEG);
	case EXPR_NOT:  ASM_UNIOP(INSN_NOT);
	
	case EXPR_AND:  ASM_BINOP(INSN_AND);
	case EXPR_OR:   ASM_BINOP(INSN_OR);
	case EXPR_XOR:  ASM_BINOP(INSN_XOR);
	case EXPR_LAND: ASM_BINOP(INSN_LAND);
	case EXPR_LOR:  ASM_BINOP(INSN_LOR);
	case EXPR_EQ:   ASM_BINOP(INSN_EQ);
	case EXPR_NEQ:  ASM_BINOP(INSN_NEQ);
	case EXPR_LE:   ASM_BINOP(INSN_LE);
	case EXPR_LT:   ASM_BINOP(INSN_LT);
	case EXPR_GE:   ASM_BINOP(INSN_GE);
	case EXPR_GT:   ASM_BINOP(INSN_GT);
	case EXPR_ADD:  ASM_BINOP(INSN_ADD);
	case EXPR_SUB:  ASM_BINOP(INSN_SUB);
	case EXPR_MUL:  ASM_BINOP(INSN_MUL);
	case EXPR_DIV:  ASM_BINOP(INSN_DIV);
	case EXPR_MOD:  ASM_BINOP(INSN_MOD);
	
	case EXPR_MINUS:
		break;
	case EXPR_IFTE: // TODO
		r0 = ASM_GenExpr(a, c, e->v.tern_op.op1);
		ASM_Push(a, INSN_JZ, r0, 0, 0); // label1
		r1 = ASM_GenExpr(a, c, e->v.tern_op.op1);
		ASM_Push(a, INSN_JMP, 0, 0, 0); // label2
		// label1
		r2 = ASM_GenExpr(a, c, e->v.tern_op.op1);
		// label2
		break;
	case EXPR_DEREF:
		break;
	case EXPR_ADDR:
		break;
	default:
		break;
	}
	
	return r0;
}

void ASM_GenStmt(ASM* a, Context* c, Stmt* s)
{
	assert(a);
	assert(c);
	assert(s);
	
	switch (s->type)
	{
	case STMT_NOTHING:
		break;
	case STMT_DECL:
		break;
	case STMT_EXPR:
		ASM_GenExpr(a, c, s->v.expr);
		break;
	case STMT_WHILE:
		break;
	case STMT_DO:
		break;
	case STMT_FO::
		break;
	case STMT_IF:
		break;
	case STMT_RETURN:
		break;
	case STMT_BLOCK:
		break;
	}
}
