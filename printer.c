/*
 *   A Tiny C to SPIM Compiler
 *   Copyright (C) 2012 Thomas GREGOIRE, Quentin SANTOS
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
 */

#include "printer.h"
#include <stdio.h>

void ExprList_Print(ExprList* l)
{
	Expr_Print(l->head);
	l = l->tail;
	while (l)
	{
		printf(", ");
		Expr_Print(l->head);
		l = l->tail;
	}
}

#define PRINT_BINOP(CODE, NAME)			\
	case EXPR_##CODE :			\
	printf(#NAME);				\
	printf("(");				\
	Expr_Print(e->v.bin_op.left);		\
	printf(", ");				\
	Expr_Print(e->v.bin_op.right);		\
	printf(")");				\
	break;

void Expr_Print(Expr* e)
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
		ExprList_Print(e->v.call.params);
		printf(")");
		break;
	case EXPR_AFF:
		printf("Aff(%s, ", e->v.aff.name);
		Expr_Print(e->v.aff.expr);
		printf(")");
		break;
	case EXPR_VAR:
		printf("Var(%s)", e->v.var.name);
		break;
		PRINT_BINOP(EQ,  Eq );
		PRINT_BINOP(NEQ, Neq);
		PRINT_BINOP(LE,  Le );
		PRINT_BINOP(LT,  Lt );
		PRINT_BINOP(GE,  Ge );
		PRINT_BINOP(GT,  Gt );
		PRINT_BINOP(ADD, Add);
		PRINT_BINOP(SUB, Sub);
		PRINT_BINOP(MUL, Mul);
		PRINT_BINOP(DIV, Div);
		PRINT_BINOP(MOD, Mod);
	case EXPR_MINUS:
		printf("Minus(");
		Expr_Print(e->v.uni_op);
		printf(")");
		break;
	case EXPR_DEREF:
		printf("Deref(");
		Expr_Print(e->v.uni_op);
		printf(")");
		break;
	case EXPR_ADDR:
		printf("Addr(");
		Expr_Print(e->v.uni_op);
		printf(")");
		break;
	case EXPR_IFTE:
		printf("Ifte(");
		Expr_Print(e->v.tern_op.op1);
		printf(", ");
		Expr_Print(e->v.tern_op.op2);
		printf(", ");
		Expr_Print(e->v.tern_op.op3);
		printf(")");
		break;
	default:
		break;
	}
}

void Type_Print(Type* t)
{
	if (!t)
	{
		printf("NULL");
		return;
	}
	switch (t->type)
	{
	case TYPE_VOID:
		printf("void");
		break;
	case TYPE_CHAR:
		printf("char");
		break;
	case TYPE_INT:
		printf("int");
		break;
	case TYPE_PTR:
		Type_Print(t->v.ptr);
		printf("*");
		break;
	default:
		break;
	}
}

void StmtList_Print(StmtList* l)
{
	if (!l)
	{
		printf("NULL");
		return;
	}
	if (l)
	{
		Stmt_Print(l->head);
		l = l->tail;
		while (l)
		{
			printf(", ");
			Stmt_Print(l->head);
			l = l->tail;
		}
	}
}

void Stmt_Print(Stmt* s)
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
		Type_Print(s->v.decl.t);
		printf(", %s, ", s->v.decl.name);
		Expr_Print(s->v.decl.val);
		printf(")");
		break;
	case STMT_EXPR:
		Expr_Print(s->v.expr);
		break;
	case STMT_WHILE:
		printf("While(");
		Expr_Print(s->v.whilez.cond);
		printf(", ");
		Stmt_Print(s->v.whilez.stmt);
		printf(")");
		break;
	case STMT_DO:
		printf("Do(");
		Stmt_Print(s->v.doz.stmt);
		printf(", ");
		Expr_Print(s->v.doz.cond);
		printf(")");
		break;
	case STMT_FOR:
		printf("For(");
		Stmt_Print(s->v.forz.a);
		printf(", ");
		Stmt_Print(s->v.forz.b);
		printf(", ");
		Stmt_Print(s->v.forz.c);
		printf(", ");
		Stmt_Print(s->v.forz.stmt);
		printf(")");
		break;
	case STMT_IF:
		printf("If(");
		Expr_Print(s->v.ifz.cond);
		printf(", ");
		Stmt_Print(s->v.ifz.iftrue);
		if (s->v.ifz.iffalse)
		{
			printf(", ");
			Stmt_Print(s->v.ifz.iffalse);
		}
		printf(")");
		break;
	case STMT_RETURN:
		printf("Return(");
		Expr_Print(s->v.expr);
		printf(")");
		break;
	case STMT_BLOCK:
		printf("Block(");
		StmtList_Print(s->v.block);
		printf(")");
	default:
		break;
	}
}

void Param_Print(Param* p)
{
	if (!p)
	{
		printf("NULL");
		return;
	}
	printf("Param(");
	Type_Print(p->type);
	printf(", %s)", p->name);
}

void ParamList_Print(ParamList* l)
{
	if (!l)
	{
		printf("NULL");
		return;
	}
	printf("ParamList(");
	Param_Print(l->head);
	l = l->tail;
	while (l)
	{
		printf(", ");
		Param_Print(l->head);
		l = l->tail;
	}
	printf(")");
}

void FunDecl_Print(FunDecl* f)
{
	if (!f)
	{
		printf("NULL");
		return;
	}
	printf("FunDecl(");
	Type_Print(f->type);
	printf(", %s, ", f->name);
	ParamList_Print(f->params);
	printf(", ");
	Stmt_Print(f->stmt);
	printf(")");
}

void Program_Print(Program* p)
{
	if (!p)
		return;
	printf("Program(");
	FunDecl_Print(p->head);
	p = p->tail;
	while (p)
	{
		printf(", ");
		FunDecl_Print(p->head);
		p = p->tail;
	}
	printf(")");
	printf("\n");
}
