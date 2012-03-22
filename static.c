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

#include "static.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * A chaque fois qu'on ajoute un symbole:
 *  1) on push son numéro sur la pile "defined"
 *  2) on incrémente le TOS de "forget"
 * A chaque début de bloc on push 0 sur la pile "forget"
 * A chaque fin de bloc, on oublie les k premiers symboles de "defined"
 * où k est le TOS de "forget", puis on pop "forget"
 */

void Block_Begin(context*);
void Block_Add(context*, u32);
void Block_End(context*);

void Static_Error(context*, position*, cstring, ...);

void Block_Begin(context* c)
{
	u32stack_push(0, &c->forget);
	c->depth++;
}

// add a symbol to the current symbol stack, counting it in the block's symbols
void Block_Add(context* c, u32 id)
{
	u32stack_push(id, &c->defined);
	c->forget->head++;
}

void Block_End(context* c)
{
	u32 k = u32stack_pop(&c->forget);
	while (k)
	{
		u32 id = u32stack_pop(&c->defined);
		c->st[id].isDeclared = false; // on oublie
		c->st[id].isDefined  = false;
		k--;
	}
	c->depth--;
}

void Check_Expr(Expr* e, context* c)
{
	HashTable* ht = c->ht;
	symbol*    st = c->st;
	string     name;
	u32        k;
	switch (e->type)
	{
	case EXPR_FUN_CALL:
		name = e->v.call.name;
		k = HashTable_find(ht, name);
		if (!st[k].isDeclared)
		{
			Static_Error(c, &e->pos, "function %s is undeclared", name);
		}
		Check_ExprList(e->v.call.params, c);
		if (st[k].isDeclared)
			Check_TypeParams(st[k].v.f, e, c);
		break;
	case EXPR_AFF:
		name = e->v.aff.name;
		k = HashTable_find(ht, name);
		if (!st[k].isDeclared)
		{
			Static_Error(c, &e->pos, "variable %s is undeclared", name);
		}
		Check_Expr(e->v.aff.expr, c);
		Check_TypeExpr(st[k].v.t, e->v.aff.expr, c);
		break;
	case EXPR_VAR:
		name = e->v.var.name;
		k = HashTable_find(ht, name);
		if (!st[k].isDeclared)
		{
			Static_Error(c, &e->pos, "variable %s is undeclared", name);
		}
		break;
	case EXPR_NEG:
	case EXPR_MINUS:
		Check_Expr(e->v.uni_op, c);
		break;
	case EXPR_CAST:
		/* XXX: typage  */
		Check_Expr(e->v.uni_op, c);
		break;
	case EXPR_ADDR:
		/* XXX: typage */
		Check_Expr(e->v.uni_op, c);
		break;
	case EXPR_EQ:
	case EXPR_NEQ:
	case EXPR_LE:
	case EXPR_LT:
	case EXPR_GE:
	case EXPR_GT:
	case EXPR_ADD:
	case EXPR_SUB:
	case EXPR_MUL:
	case EXPR_DIV:
	case EXPR_MOD:
		Check_Expr(e->v.bin_op.left,  c);
		Check_Expr(e->v.bin_op.right, c);
		break;
	case EXPR_IFTE:
		Check_Expr(e->v.tern_op.op1, c);
		Check_Expr(e->v.tern_op.op2, c);
		Check_Expr(e->v.tern_op.op3, c);
		break;
	default:
		/* No problem... */
		break;
	}
}

void Check_ExprList(ExprList* l, context* c)
{
	while (l)
	{
		Check_Expr(l->head, c);
		l = l->tail;
	}
}

void Check_Stmt(Stmt* s, context* c)
{
	HashTable* ht = c->ht;
	symbol*    st = c->st;
	string     name;
	u32        k;
	switch (s->type)
	{
	case STMT_DECL:
		name = s->v.decl.name;
		k = HashTable_find(ht, name);
		if (st[k].isDeclared)
		{
			Static_Error(c, &s->v.decl.pos, "redeclaration of %s", name);
			Static_Error(c, &s->v.decl.pos, "previous declaration was here: Line %d, character %d", st[k].pos->first_line, st[k].pos->first_column);
		}
		else
		{
			Block_Add(c, k);
			st[k].isDeclared = true;
			st[k].isFun      = false;
			st[k].pos        = &s->v.decl.pos;
			st[k].v.t        = s->v.decl.t;
			if (s->v.decl.val)
			{
				Check_Expr(s->v.decl.val, c);
				st[k].isDefined = true;
				Check_TypeExpr(s->v.decl.t, s->v.decl.val, c);
			}
			else
				st[k].isDefined = false;
		}
		break;
	case STMT_EXPR:
	case STMT_RETURN:
		/* XXX */
		Check_Expr(s->v.expr, c);
		break;
	case STMT_WHILE:
		Check_Expr(s->v.whilez.cond, c);
		Check_Stmt(s->v.whilez.stmt, c);
		break;
	case STMT_DO:
		Check_Stmt(s->v.doz.stmt, c);
		Check_Expr(s->v.doz.cond, c);
		break;
	case STMT_FOR:
		Check_Stmt(s->v.forz.a,    c);
		Check_Stmt(s->v.forz.b,    c);
		Check_Stmt(s->v.forz.c,    c);
		Check_Stmt(s->v.forz.stmt, c);
		break;
	case STMT_IF:
		Check_Expr(s->v.ifz.cond, c);
		Check_Stmt(s->v.ifz.iftrue, c);
		Check_Stmt(s->v.ifz.iffalse, c);
		break;
	case STMT_BLOCK:
		Block_Begin(c);
		Check_StmtList(s->v.block, c);
		Block_End(c);
		break;
	default:
		break;
	}
}

void Check_StmtList(StmtList* l, context* c)
{
	while (l)
	{
		Check_Stmt(l->head, c);
		l = l->tail;
	}
}

void Check_Param(Param* p, context* c)
{
	HashTable* ht   = c->ht;
	symbol*    st   = c->st;
	string     name = p->name;
	u32 k = HashTable_find(ht, name);
	if (st[k].isDeclared)
	{
		Static_Error(c, &p->pos, "redeclaration of %s", name);
		Static_Error(c, &p->pos, "previous declaration was here: Line %d, character %d", st[k].pos->first_line, st[k].pos->first_column);
	}
	else
	{
		Block_Add(c, k);
		st[k].isDeclared = true;
		st[k].isDefined  = false;
		st[k].isFun      = false;
		st[k].pos        = &p->pos;
		st[k].v.t        = p->type;
	}
}

void Check_ParamList(ParamList* l, context* c)
{
	while (l)
	{
		Check_Param(l->head, c);
		l = l->tail;
	}
}

void Check_FunDecl(FunDecl* fd, context* c)
{
	HashTable* ht   = c->ht;
	symbol*    st   = c->st;
	string     name = fd->name;
	u32 k = HashTable_find(ht, name);
	if (st[k].isDefined)
	{
		Static_Error(c, &fd->pos, "redeclaration of %s", name);
		Static_Error(c, &fd->pos, "previous declaration was here: Line %d, character %d", st[k].pos->first_line, st[k].pos->first_column);
	}
	else
	{
		Block_Add(c, k);
		st[k].isDeclared = true;
		st[k].isFun      = true;
		st[k].pos        = &fd->pos;
		st[k].v.f        = fd;
		Block_Begin(c);
		Check_ParamList(fd->params, c);
		Check_Stmt(fd->stmt, c);
		Block_End(c);
	}
}

void Check_Program(Program* l, context* c)
{
	Block_Begin(c);
	while (l)
	{
		Check_FunDecl(l->head, c);
		l = l->tail;
	}
	Block_End(c);
}

context* Context_New(u32 size)
{
	context* c = (context*) malloc(sizeof(context));
	assert(c);
	c->ht = HashTable_new(size);
	c->st = (symbol*) malloc(sizeof(symbol) * size);
	while (size)
	{
		c->st[size - 1].isDeclared = false;
		c->st[size - 1].isDefined  = false;
		size --;
	}
	c->err = false;
	c->depth = 0;
	c->defined = NULL;
	c->forget  = NULL;
	return c;
}

void Context_Delete(context* c)
{
	/* XXX: virer les stacks */
	HashTable_delete(c->ht);
	free(c->st);
	free(c);
}

void Static_Error(context* c, position* pos, cstring format, ...)
{
	va_list va;
	va_start(va, format);
	fprintf(stderr, "Line %d, character %d: ", pos->first_line, pos->first_column);
	vfprintf(stderr, format, va);
	fprintf(stderr, "\n");
	va_end(va);
	c->err = true;
}

/* Typage */

void Check_TypeExpr(Type* t, Expr* e, context* c)
{
	(void) t;
	(void) e;
	(void) c;
	return;
	switch (e->type)
	{
	case EXPR_INTEGER:
		break;
	case EXPR_FUN_CALL:
		break;
	case EXPR_AFF:
		break;
	case EXPR_VAR:
		break;
	case EXPR_NEG:
		break;
	case EXPR_EQ:
	case EXPR_NEQ:
	case EXPR_LE:
	case EXPR_LT:
	case EXPR_GE:
	case EXPR_GT:
	case EXPR_ADD:
	case EXPR_SUB:
	case EXPR_MUL:
	case EXPR_DIV:
	case EXPR_MOD:
		break;
	case EXPR_MINUS:
	case EXPR_CAST:
	case EXPR_ADDR:
		break;
	case EXPR_IFTE:
		break;
	default:
		break;
	}
}

void Check_TypeParams(FunDecl* fd, Expr* e, context* c)
{
	ParamList* p = fd->params;
	ExprList*  l = e->v.call.params;
	while (l && p)
	{
		Check_TypeExpr(p->head->type, l->head, c);
		p = p->tail;
		l = l->tail;
	}
	if (l)
		Static_Error(c, &e->pos, "too many arguments to function %s", fd->name);
	else if (p)
		Static_Error(c, &e->pos, "too few arguments to function %s", fd->name);
}
