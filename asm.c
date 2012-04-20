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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

ASM* ASM_New(Context* c)
{
	ASM* ret    = (ASM*) malloc(sizeof(ASM));
	assert(ret);
	
	ret->n_code = 0;
	ret->a_code = 1024;
	ret->code   = (Instr*) malloc(sizeof(Instr) * ret->a_code);
	assert(ret->code);
	
	ret->n_regs = 1;
	
	ret->n_labels = 0;
	ret->a_labels = 1024;
	ret->labels   = (u32*) malloc(sizeof(Instr) * ret->a_labels);
	assert(ret->labels);
	
	ret->funCalls = (u32stack**) malloc(sizeof(u32stack*) * c->n_symbs);
	assert(ret->funCalls);
	memset(ret->funCalls, 0, sizeof(u32stack*) * c->n_symbs);
	
	return ret;
}

void ASM_Delete(ASM* a)
{
	assert(a);
/*	
	for (u32 i = 0; i < a->n_code; i++)
	{
		Set_Delete(a->code[i].s.use);
		Set_Delete(a->code[i].s.def);
		Set_Delete(a->code[i].s.in);
		Set_Delete(a->code[i].s.out);
	}
*/	
	free(a->code);
	free(a);
}

void ASM_Push(ASM* a, ASM_INSN insn, u32 r0, u32 r1, u32 r2)
{
	assert(a);
	
	if (a->n_code == a->a_code)
	{
		a->a_code *= 2;
		a->code = (Instr*)realloc(a->code, sizeof(Instr) * a->a_code);
		assert(a->code);
	}
	a->code[a->n_code].insn = insn;
	a->code[a->n_code].v.r.r0 = r0;
	a->code[a->n_code].v.r.r1 = r1;
	a->code[a->n_code].v.r.r2 = r2;
	a->n_code++;
}

void ASM_PushList(ASM* a, ASM_INSN insn, u32stack* reg)
{
	assert(a);
	
	if (a->n_code == a->a_code)
	{
		a->a_code *= 2;
		a->code = (Instr*)realloc(a->code, sizeof(Instr) * a->a_code);
		assert(a->code);
	}
	a->code[a->n_code].insn = insn;
	a->code[a->n_code].v.p = reg;
	a->n_code++;
}

u32 ASM_NewReg(ASM* a, Context* c)
{
	assert(a);
	assert(c);
	
	if (c->cur_fun)
		u32stack_push(&c->st[c->cur_fun->id].usedRegs, a->n_regs);
	return a->n_regs++;
}

u32 ASM_NewLabel(ASM* a)
{
	assert(a);
	
	if (a->n_labels == a->a_labels)
	{
		a->a_labels *= 2;
		a->labels = (u32*)realloc(a->labels, sizeof(u32) * a->a_code);
		assert(a->labels);
	}
	return a->n_labels++;
}

void ASM_LabelPos(ASM* a, u32 label)
{
	assert(a);
	
	while (label >= a->a_labels)
	{
		a->a_labels *= 2;
		a->labels = (u32*)realloc(a->labels, sizeof(u32) * a->a_labels);
		assert(a->labels);
	}
	a->labels[label] = a->n_code;
	ASM_Push(a, INSN_LBL, label, 0, 0);
}

#define ASM_UNIOP(INSTR)                           \
	r0 = ASM_NewReg(a, c);                        \
	r1 = ASM_GenExpr(a, c, e->v.uni_op);       \
	ASM_Push(a, INSTR, r0, r1, 0);             \
	break;                                     \

#define ASM_BINOP(INSTR)                           \
	r0 = ASM_NewReg(a, c);                        \
	r1 = ASM_GenExpr(a, c, e->v.bin_op.left);  \
	r2 = ASM_GenExpr(a, c, e->v.bin_op.right); \
	ASM_Push(a, INSTR, r0, r1, r2);            \
	break;                                     \

u32 ASM_GenExpr(ASM* a, Context* c, Expr* e)
{
	assert(a);
	assert(c);
	assert(e);
	
	u32stack* regs = NULL;
	ExprList* es;
	u32 r0        = 0;
	u32 r1;
	u32 r2;
	u32 r3;
	u32 l0;
	u32 l1;
	
	switch (e->type)
	{
	case EXPR_INTEGER:
		r0 = ASM_NewReg(a, c);
		ASM_Push(a, INSN_SET, r0, e->v.i, 0);
		break;
	case EXPR_FUN_CALL:
		es = e->v.call.params;
		while (es)
		{
			r0 = ASM_GenExpr(a, c, es->head);
			r1 = ASM_NewReg(a, c);
			ASM_Push(a, INSN_MOV, r1, r0, 0);
			u32stack_push(&regs, r1);
			es = es->tail;
		}
		u32stack_push(&(a->funCalls[e->v.call.id]), a->n_code);
		ASM_PushList(a, INSN_CALL, regs);
		regs = NULL;
		r0 = c->st[e->v.call.id].reg;
		break;
	case EXPR_AFF:
		r0 = c->st[e->v.aff.id].reg;
		r1 = ASM_GenExpr(a, c, e->v.aff.expr);
		ASM_Push(a, INSN_MOV, r0, r1, 0);
		break;
	case EXPR_VAR:
		r0 = c->st[e->v.aff.id].reg;
		break;
	
	case EXPR_NOT:  ASM_UNIOP(INSN_NOT);
	case EXPR_AND:  ASM_BINOP(INSN_AND);
	case EXPR_OR:   ASM_BINOP(INSN_OR);
	case EXPR_XOR:  ASM_BINOP(INSN_XOR);
	case EXPR_LNOT: ASM_UNIOP(INSN_LNOT);
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
		r0 = ASM_GenExpr(a, c, e->v.uni_op);
		r1 = ASM_NewReg(a, c);
		ASM_Push(a, INSN_SET, r0,  0, 0);
		ASM_Push(a, INSN_SUB, r0, r1, 0);
		break;
	case EXPR_IFTE:
		r3 = ASM_NewReg(a, c);
		l0 = ASM_NewLabel(a);
		l1 = ASM_NewLabel(a);
		
		r0 = ASM_GenExpr(a, c, e->v.tern_op.op1);
		ASM_Push(a, INSN_JZ, r0, l0, 0);
		r1 = ASM_GenExpr(a, c, e->v.tern_op.op2);
		ASM_Push(a, INSN_MOV, r3, r1, 0);
		ASM_Push(a, INSN_JMP, l1, 0, 0);
		ASM_LabelPos(a, l0);
		r2 = ASM_GenExpr(a, c, e->v.tern_op.op3);
		ASM_Push(a, INSN_MOV, r3, r2, 0);
		ASM_LabelPos(a, l1);
		r0 = r3;
		break;
	case EXPR_DEREF: // TODO
		break;
	case EXPR_ADDR: // TODO
		break;
	}
	
	return r0;
}

void ASM_GenStmt(ASM* a, Context* c, Stmt* s)
{
	assert(a);
	assert(c);
	assert(s);
	
	u32 r0;
	u32 r1;
	u32 l0;
	u32 l1;
	StmtList* l;
	
	switch (s->type)
	{
	case STMT_NOTHING:
		break;
	case STMT_DECL:
		r0 = ASM_NewReg(a, c);
		c->st[s->v.decl.id].reg = r0;
		if (s->v.decl.val)
		{
			r1 = ASM_GenExpr(a, c, s->v.decl.val);
			ASM_Push(a, INSN_MOV, r0, r1, 0);
		}
		break;
	case STMT_EXPR:
		ASM_GenExpr(a, c, s->v.expr);
		break;
	case STMT_WHILE:
		l0 = ASM_NewLabel(a);
		l1 = ASM_NewLabel(a);
		
		ASM_LabelPos(a, l0);
		r0 = ASM_GenExpr(a, c, s->v.whilez.cond);
		ASM_Push(a, INSN_JZ, r0, l1, 0);
		ASM_GenStmt(a, c, s->v.whilez.stmt);
		ASM_Push(a, INSN_JMP, l0, 0, 0);
		ASM_LabelPos(a, l1);
		break;
	case STMT_DO:
		l0 = ASM_NewLabel(a);
		l1 = ASM_NewLabel(a);
		
		ASM_LabelPos(a, l0);
		ASM_GenStmt(a, c, s->v.doz.stmt);
		r0 = ASM_GenExpr(a, c, s->v.doz.cond);
		ASM_Push(a, INSN_JZ, r0, l1, 0);
		ASM_Push(a, INSN_JMP, l0, 0, 0);
		ASM_LabelPos(a, l1);
		break;
	case STMT_FOR:
		l0 = ASM_NewLabel(a);
		l1 = ASM_NewLabel(a);
		
		ASM_GenStmt(a, c, s->v.forz.a);
		ASM_LabelPos(a, l0);
		r0 = ASM_GenExpr(a, c, s->v.forz.b);
		ASM_Push(a, INSN_JZ, r0, l1, 0);
		ASM_GenStmt(a, c, s->v.forz.stmt);
		ASM_GenStmt(a, c, s->v.forz.c);
		ASM_Push(a, INSN_JMP, l0, 0, 0);
		ASM_LabelPos(a, l1);
		break;
	case STMT_IF:
		l0 = ASM_NewLabel(a);
		l1 = ASM_NewLabel(a);
		
		r0 = ASM_GenExpr(a, c, s->v.ifz.cond);
		ASM_Push(a, INSN_JZ, r0, l0, 0);
		ASM_GenStmt(a, c, s->v.ifz.iftrue);
		ASM_Push(a, INSN_JMP, l1, 0, 0);
		ASM_LabelPos(a, l0);
		ASM_GenStmt(a, c, s->v.ifz.iffalse);
		ASM_LabelPos(a, l1);
		break;
	case STMT_RETURN:
		assert(c->cur_fun);
		r0 = ASM_GenExpr(a, c, s->v.expr);
		ASM_Push(a, INSN_MOV, VREG_RETURN, r0, 0);
		break;
	case STMT_BLOCK:
		l = s->v.block;
		while (l)
		{
			ASM_GenStmt(a, c, l->head);
			l = l->tail;
		}
		break;
	}
}

void ASM_GenFun (ASM* a, Context* c, FunDecl* f)
{
	assert(a);
	assert(c);
	assert(f);
	
	FunDecl* oldfun = c->cur_fun;
	c->cur_fun = f;
	u32 l = ASM_NewLabel(a);
	c->st[f->id].label = l;
	
	ParamList* p = f->params;
	while (p)
	{
		u32 r0 = ASM_NewReg(a, c);
		c->st[p->head->id].reg = r0;
		u32stack_push(&c->st[f->id].params, r0);
		p = p->tail;
	}
	
	ASM_LabelPos(a, l);
	a->code[a->n_code-1].v.r.r1 = f->id;
	ASM_GenStmt(a, c, f->stmt);
	ASM_Push(a, INSN_RET, f->id, 0, 0);
	
	c->cur_fun = oldfun;
}

void ASM_GenProgram(ASM* a, Context* c, Program* p)
{
	ASM_Push(a, INSN_CALL, 0, 0, 0);
	ASM_Push(a, INSN_STOP, 0, 0, 0);
	
	while (p)
	{
		ASM_GenFun(a, c, p->head);
		p = p->tail;
	}
	
	u32stack_push(&(a->code[0].v.p), c->st[c->main].label);
	for (u32 i = 0; i < c->n_symbs; i++)
	{
		u32stack* calls = a->funCalls[i];
		while (calls)
		{
			u32 pos = calls->head;
			u32stack_push(&(a->code[pos].v.p), c->st[i].label);
			calls = calls->tail;
		}
	}
}
