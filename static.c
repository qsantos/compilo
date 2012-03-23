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

/* CONTEXT ALTERATION */
context* Context_New(u32 size)
{
	context* c = (context*) malloc(sizeof(context));
	assert(c);
	
	c->ht = HashTable_new(size);
	
	c->ht2st = (u32stack*)malloc(sizeof(u32stack*) * size);
	memset(c->ht2st, 0, sizeof(u32stack*) * size);
	
	c->st = (symbol*) malloc(sizeof(symbol) * size);
	while (size)
	{
		c->st[size - 1].isDeclared = false;
		c->st[size - 1].isDefined  = false;
		size --;
	}
	
	c->err = false;
	c->defined = NULL;
	c->forget  = NULL;
	return c;
}
void Context_Delete(context* c)
{
	u32stack_delete(&c->forget);
	u32stack_delete(&c->defined);
	for (u32 i = 0; i < c->ht->size; i++)
		u32stack_delete(c->ht2st[i]);
	free(c->ht2st);
	HashTable_delete(c->ht);
	free(c->st);
	free(c);
}
void Context_BeginBlock(context* c)
{
	u32stack_push(0, &c->forget);
}
void Context_Define(context* c, u32 id)
{
	u32stack_push(id, &c->defined);
	c->forget->head++;
}
void Context_EndBlock(context* c)
{
	u32 k = u32stack_pop(&c->forget);
	while (k)
	{
		u32 id = u32stack_pop(&c->defined);
		c->st[id].isDeclared = false;
		c->st[id].isDefined  = false;
		k--;
	}
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

/* TYPE COMPARISON */
void Type_Check(Type* t1, Type* t2, position* pos, context* c)
{
	if (!Type_Comp(t1, t2))
	{
		fprintf(stderr, "Line %d, character %d: types '", pos->first_line, pos->first_column);
		Type_Print(stderr, t1);
		fprintf(stderr, "' and '");
		Type_Print(stderr, t2);
		fprintf(stderr, "' mismatch.\n");
		c->err = true;
	}
}


/* SYMBOLS DECLARATION, DEFINITION AND TYPE CHECK */
void Check_Expr(Expr* e, context* c)
{
	HashTable* ht = c->ht;
	symbol*    st = c->st;
	string     name;
	u32        k;
	Type*      t;
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
		if (!c->err)
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
		if (!c->err)
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
	case EXPR_DEREF:
	case EXPR_ADDR:
		t = Type_Expr(e->v.uni_op, c);
		Check_Expr(e, c);
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

void Check_Stmt(Stmt* s, bool needRet, context* c)
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
			Context_Define(c, k);
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
		Check_Expr(s->v.expr, c);
		break;
	case STMT_RETURN:
		Check_Expr(s->v.expr, c);
		if (!c->err)
			Check_TypeExpr(c->cur_fun->type, s->v.expr, c);
		break;
	case STMT_WHILE:
		Check_Expr(s->v.whilez.cond, c);
		Check_Stmt(s->v.whilez.stmt, false, c);
		break;
	case STMT_DO:
		Check_Stmt(s->v.doz.stmt, false, c);
		Check_Expr(s->v.doz.cond, c);
		break;
	case STMT_FOR:
		Check_Stmt(s->v.forz.a,    false, c);
		Check_Stmt(s->v.forz.b,    false, c);
		Check_Stmt(s->v.forz.c,    false, c);
		Check_Stmt(s->v.forz.stmt, false, c);
		break;
	case STMT_IF:
		Check_Expr(s->v.ifz.cond, c);
		if (s->v.ifz.iffalse)
		{
			Check_Stmt(s->v.ifz.iftrue,  needRet, c);
			Check_Stmt(s->v.ifz.iffalse, needRet, c);
		}
		else
			Check_Stmt(s->v.ifz.iftrue, false, c);		
		break;
	case STMT_BLOCK:
		Context_BeginBlock(c);
		Check_StmtList(s->v.block, needRet, c);
		Context_EndBlock(c);
		break;
	default:
		break;
	}

	if (needRet && s->type != STMT_BLOCK && s->type != STMT_RETURN && s->type != STMT_IF)
		Static_Error(c, &c->cur_fun->pos, "no return statement at end of non-void function '%s'", c->cur_fun->name);
}

void Check_StmtList(StmtList* l, bool needRet, context* c)
{
	if (!l && needRet)
		Static_Error(c, &c->cur_fun->pos, "no return statement at end of non-void function '%s'", c->cur_fun->name);
	while (l)
	{
		Check_Stmt(l->head, !l->tail && needRet, c);
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
		Context_Define(c, k);
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
		Context_Define(c, k);
		c->cur_fun = fd;
		st[k].isDeclared = true;
		st[k].isFun      = true;
		st[k].pos        = &fd->pos;
		st[k].v.f        = fd;
		Context_BeginBlock(c);
		Check_ParamList(fd->params, c);
		Check_Stmt(fd->stmt, !Type_Comp(fd->type, &TVoid), c);
		Context_EndBlock(c);
	}
}

void Check_Program(Program* l, context* c)
{
	Context_BeginBlock(c);
	while (l)
	{
		Check_FunDecl(l->head, c);
		l = l->tail;
	}
	Context_EndBlock(c);
}

/* Typage */

Type* Type_Expr(Expr* e, context* c)
{
	HashTable* ht   = c->ht;
	symbol*    st   = c->st;
	u32 k;
	Type* t;
	switch (e->type)
	{
	case EXPR_INTEGER:
		return &TInt;
	case EXPR_FUN_CALL:
		k = HashTable_find(ht, e->v.call.name);
		return st[k].v.f->type;
	case EXPR_AFF:
		k = HashTable_find(ht, e->v.aff.name);
		t = st[k].v.t;
		Type_Check(t, Type_Expr(e->v.aff.expr, c), &e->pos, c);
		return t;
	case EXPR_VAR:
		k = HashTable_find(ht, e->v.var.name);
		return st[k].v.t;
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
	case EXPR_NEG:
		t = Type_Expr(e->v.bin_op.left, c);
		Type_Check(t, Type_Expr(e->v.bin_op.right, c), &e->pos, c);
		return t;
	case EXPR_MINUS:
		return Type_Expr(e->v.uni_op, c);
	case EXPR_IFTE:
		t = Type_Expr(e->v.tern_op.op1, c);
		t = Type_Expr(e->v.tern_op.op2, c);
		Type_Check(t, Type_Expr(e->v.tern_op.op3, c), &e->pos, c);
		return t;
	case EXPR_DEREF:
		t = Type_Expr(e->v.uni_op, c);
		if (t->type != TYPE_PTR)
		{
			Static_Error(c, &e->pos, "dereferencing a non-pointer variable");
			return t->v.ptr;
		}
		else
			return &TInt;
	case EXPR_ADDR:
		return Type_Ptr(Type_Expr(e->v.uni_op, c));
	default:
		return &TInt;
	}
}

void Check_TypeExpr(Type* t, Expr* e, context* c)
{
	Type_Check(t, Type_Expr(e, c), &e->pos, c);
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
	else if (p && !Type_Comp(p->head->type, &TVoid))
		Static_Error(c, &e->pos, "too few arguments to function %s", fd->name);
}
