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

typedef struct u32stack u32stack;

struct u32stack
{
	u32       head;
	u32stack* tail;
};

void u32stack_push(u32, u32stack**);
u32  u32stack_pop(u32stack**);

void u32stack_push(u32 e, u32stack** s)
{
	u32stack* t = (u32stack*) malloc(sizeof(u32stack));
	assert(t);
	t->head = e;
	t->tail = *s;
	*s = t;
}

u32 u32stack_pop(u32stack** s)
{
	u32 tos     = (*s)->head;
	u32stack* t = (*s)->tail;
	free(*s);
	*s = t;
	return tos;
}

/*
 * /!\ DENREE RARE: commentaire /!\
 *
 * A chaque fois qu'on ajoute un symbole:
 *  1) on push son numéro sur la pile "defined"
 *  2) on incrémente le TOS de "forget"
 * A chaque début de bloc on push 0 sur la pile "forget"
 * A chaque fin de bloc, on oublie les k premiers symboles de "defined"
 * où k est le TOS de "forget", puis on pop "forget"
 */

u32stack* defined;
u32stack* forget;

void Block_Begin(void);
void Block_Add(u32);
void Block_End(context*);

void Static_Error(context*, position*, cstring, ...);

void Block_Begin(void)
{
	u32stack_push(0, &forget);
}

void Block_Add(u32 id)
{
	u32stack_push(id, &defined);
	forget->head++;
}

void Block_End(context* c)
{
	u32 k = u32stack_pop(&forget);
	while (k)
	{
		u32 id = u32stack_pop(&defined);
		c->st[id].isDeclared = false; // on oublie
		c->st[id].isDefined  = false;
		k --;
	}
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
			Static_Error(c, &e->v.call.pos, "function %s is undeclared", name);
		}
		Check_ExprList(e->v.call.params, c);
		break;
	case EXPR_AFF:
		name = e->v.aff.name;
		k = HashTable_find(ht, name);
		if (!st[k].isDeclared)
		{
			Static_Error(c, &e->v.aff.pos, "variable %s is undeclared", name);
		}
		Check_Expr(e->v.aff.expr, c);
		break;
	case EXPR_VAR:
		name = e->v.var.name;
		k = HashTable_find(ht, name);
		if (!st[k].isDeclared)
		{
			Static_Error(c, &e->v.var.pos, "variable %s is undeclared", name);
		}
		break;
	case EXPR_NEG:
	case EXPR_MINUS:
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
			Block_Add(k);
			st[k].isDeclared = true;
			st[k].isFun      = false;
			st[k].pos        = &s->v.decl.pos;
			st[k].v.t        = s->v.decl.t;
			if (s->v.decl.val)
			{
				Check_Expr(s->v.decl.val, c);
				st[k].isDefined = true;
			}
			else
				st[k].isDefined = false;
		}
		break;
	case STMT_EXPR:
	case STMT_RETURN:
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
		Block_Begin();
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
		Block_Add(k);
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
		Block_Add(k);
		st[k].isDeclared = true;
		st[k].isFun      = true;
		st[k].pos        = &fd->pos;
		st[k].v.f        = fd;
		Block_Begin();
		Check_ParamList(fd->params, c);
		Check_Stmt(fd->stmt, c);
		Block_End(c);
	}
}

void Check_Program(Program* l, context* c)
{
	Block_Begin();
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
	return c;
}

void Context_Delete(context* c)
{
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
