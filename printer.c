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

#include "printer.h"

#include <stdio.h>
#include <assert.h>

void Print_LValue(LValue* lv)
{
	if (lv->var)
		printf("%s", lv->v.var.s);
	else
	{
		printf("*");
		Print_Expr(lv->v.e);
	}
}

void Print_ExprList(ExprList* l)
{
	if (l)
	{
		Print_Expr(l->head);
		l = l->tail;
		while (l)
		{
			printf(", ");
			Print_Expr(l->head);
			l = l->tail;
		}
	}
}

#define EXPR_BINOP(NAME)			\
	printf(#NAME);				\
	printf("(");				\
	Print_Expr(e->v.bin_op.left);		\
	printf(", ");				\
	Print_Expr(e->v.bin_op.right);		\
	printf(")");				\
	break;

void Print_Expr(Expr* e)
{
	if (!e)
	{
		printf("NULL");
		return;
	}
	switch (e->type)
	{
	case EXPR_INTEGER:
		printf("Int(%ld)", e->v.i);
		break;
	case EXPR_FUN_CALL:
		printf("Call(%s, ", e->v.call.name);
		Print_ExprList(e->v.call.params);
		printf(")");
		break;
	case EXPR_AFF:
		printf("Aff(");
		Print_LValue(e->v.aff.lv);
		printf(", ");
		Print_Expr(e->v.aff.expr);
		printf(")");
		break;
	case EXPR_VAR:
		printf("Var(%s)", e->v.var.name);
		break;
	case EXPR_EQ:   EXPR_BINOP(Eq );
	case EXPR_NEQ:  EXPR_BINOP(Neq);
	case EXPR_LE:   EXPR_BINOP(Le );
	case EXPR_LT:   EXPR_BINOP(Lt );
	case EXPR_GE:   EXPR_BINOP(Ge );
	case EXPR_GT:   EXPR_BINOP(Gt );
	case EXPR_ADD:  EXPR_BINOP(Add);
	case EXPR_SUB:  EXPR_BINOP(Sub);
	case EXPR_MUL:  EXPR_BINOP(Mul);
	case EXPR_DIV:  EXPR_BINOP(Div);
	case EXPR_MOD:  EXPR_BINOP(Mod);
	case EXPR_NOT:  EXPR_BINOP(Not);
	case EXPR_AND:  EXPR_BINOP(And);
	case EXPR_OR:   EXPR_BINOP(Or);
	case EXPR_XOR:  EXPR_BINOP(Xor);
	case EXPR_LNOT: EXPR_BINOP(LNot);
	case EXPR_LAND: EXPR_BINOP(LAnd);
	case EXPR_LOR:  EXPR_BINOP(LOr);
	case EXPR_MINUS:
		printf("Minus(");
		Print_Expr(e->v.uni_op);
		printf(")");
		break;
	case EXPR_DEREF:
		printf("Deref(");
		Print_Expr(e->v.uni_op);
		printf(")");
		break;
	case EXPR_ADDR:
		printf("Addr(%s)", e->v.var.name);
		break;
	case EXPR_IFTE:
		printf("Ifte(");
		Print_Expr(e->v.tern_op.op1);
		printf(", ");
		Print_Expr(e->v.tern_op.op2);
		printf(", ");
		Print_Expr(e->v.tern_op.op3);
		printf(")");
		break;
	}
}

void Print_StmtList(StmtList* l)
{
	if (!l)
	{
		printf("NULL");
		return;
	}
	if (l)
	{
		Print_Stmt(l->head);
		l = l->tail;
		while (l)
		{
			printf(", ");
			Print_Stmt(l->head);
			l = l->tail;
		}
	}
}

void Print_Stmt(Stmt* s)
{
	if (!s)
	{
		printf("NULL");
		return;
	}
	switch (s->type)
	{
	case STMT_NOTHING:
		printf("Nothing");
		break;
	case STMT_DECL:
		printf("Decl(");
		Print_Type(stdout, s->v.decl.t);
		printf(", %s)", s->v.decl.name);
		break;
	case STMT_EXPR:
		Print_Expr(s->v.expr);
		break;
	case STMT_WHILE:
		printf("While(");
		Print_Expr(s->v.whilez.cond);
		printf(", ");
		Print_Stmt(s->v.whilez.stmt);
		printf(")");
		break;
	case STMT_DO:
		printf("Do(");
		Print_Stmt(s->v.doz.stmt);
		printf(", ");
		Print_Expr(s->v.doz.cond);
		printf(")");
		break;
	case STMT_FOR:
		printf("For(");
		Print_Stmt(s->v.forz.a);
		printf(", ");
		Print_Expr(s->v.forz.b);
		printf(", ");
		Print_Stmt(s->v.forz.c);
		printf(", ");
		Print_Stmt(s->v.forz.stmt);
		printf(")");
		break;
	case STMT_IF:
		printf("If(");
		Print_Expr(s->v.ifz.cond);
		printf(", ");
		Print_Stmt(s->v.ifz.iftrue);
		if (s->v.ifz.iffalse)
		{
			printf(", ");
			Print_Stmt(s->v.ifz.iffalse);
		}
		printf(")");
		break;
	case STMT_RETURN:
		printf("Return(");
		Print_Expr(s->v.expr);
		printf(")");
		break;
	case STMT_BLOCK:
		printf("Block(");
		Print_StmtList(s->v.block);
		printf(")");
	}
}

void Print_Param(Param* p)
{
	if (!p)
	{
		printf("NULL");
		return;
	}
	printf("Param(");
	Print_Type(stdout, p->type);
	printf(", %s)", p->name);
}

void Print_ParamList(ParamList* l)
{
	if (!l)
	{
		printf("NULL");
		return;
	}
	printf("ParamList(");
	Print_Param(l->head);
	l = l->tail;
	while (l)
	{
		printf(", ");
		Print_Param(l->head);
		l = l->tail;
	}
	printf(")");
}

void Print_FunDecl(FunDecl* f)
{
	if (!f)
	{
		printf("NULL");
		return;
	}
	printf("FunDecl(");
	Print_Type(stdout, f->type);
	printf(", %s, ", f->name);
	Print_ParamList(f->params);
	printf(", ");
	Print_Stmt(f->stmt);
	printf(")");
}

void Print_Program(Program* p)
{
	if (!p)
		return;
	printf("Program(");
	Print_FunDecl(p->head);
	p = p->tail;
	while (p)
	{
		printf(", ");
		Print_FunDecl(p->head);
		p = p->tail;
	}
	printf(")");
	printf("\n");
}

#define ASM_BINOP(STR)                                                                  \
	printf("\t%s $%lu, $%lu, $%lu\n", STR, instr.v.r.r0, instr.v.r.r1, instr.v.r.r2); \
	break;                                                                          \

void Print_ASM(ASM* a)
{
	assert(a);
	
	u32stack* regs;
	
	for (u32 i = 0; i < a->n_code; i++)
	{
		Instr instr = a->code[i];
		switch (instr.insn)
		{
		case INSN_STOP:
			printf("\tStop\n");
			break;
		case INSN_SET: printf("\tSet  $%lu, %lu\n",  instr.v.r.r0, instr.v.r.r1); break;
		case INSN_MOV: printf("\tMov  $%lu, $%lu\n", instr.v.r.r0, instr.v.r.r1); break;
		case INSN_MRD: printf("\tMld  $%lu, $%lu\n", instr.v.r.r0, instr.v.r.r1); break;
		case INSN_MWR: printf("\tMwr  $%lu, $%lu\n", instr.v.r.r0, instr.v.r.r1); break;
		case INSN_RGA: printf("\tRga  $%lu, $%lu\n", instr.v.r.r0, instr.v.r.r1); break;
		case INSN_NOT:  ASM_BINOP("Not ");
		case INSN_AND:  ASM_BINOP("And ");
		case INSN_OR:   ASM_BINOP("Or  ");
		case INSN_XOR:  ASM_BINOP("Xor ");
		case INSN_LNOT: ASM_BINOP("Lnot");
		case INSN_LAND: ASM_BINOP("Land");
		case INSN_LOR:  ASM_BINOP("Lor ");
		case INSN_EQ:   ASM_BINOP("Eq  ");
		case INSN_NEQ:  ASM_BINOP("Neq ");
		case INSN_LE:   ASM_BINOP("Le  ");
		case INSN_LT:   ASM_BINOP("Lt  ");
		case INSN_GE:   ASM_BINOP("Ge  ");
		case INSN_GT:   ASM_BINOP("Gt  ");
		case INSN_ADD:  ASM_BINOP("Add ");
		case INSN_SUB:  ASM_BINOP("Sub ");
		case INSN_MUL:  ASM_BINOP("Mul ");
		case INSN_DIV:  ASM_BINOP("Div ");
		case INSN_MOD:  ASM_BINOP("Mod ");
		case INSN_JMP:
			printf("\tJmp  .%lu\n", instr.v.r.r0);
			break;
		case INSN_JZ:
			printf("\tJz   $%lu, .%lu\n", instr.v.r.r1, instr.v.r.r0);
			break;
		case INSN_JNZ:
			printf("\tJnz  $%lu, .%lu\n", instr.v.r.r1, instr.v.r.r0);
			break;
		case INSN_CALL:
			regs = instr.v.p;
			assert(regs);
			printf("\tCall .%lu, %lu", regs->head, regs->tail->head);
			regs = regs->tail->tail;
			while (regs)
			{
				printf(", $%lu", regs->head);
				regs = regs->tail;
			}
			printf("\n");
			break;
		case INSN_RET:
			printf("\tRet %lu\n", instr.v.r.r1);
			break;
		case INSN_LBL:
			if (instr.v.r.r1)
				printf("Function ");
			printf(".%lu\n", instr.v.r.r0);
			break;
		}
	}
}

