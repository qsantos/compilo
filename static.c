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

#include "static.h"

#include <string.h>
#include <assert.h>
#include <string.h>

static void Check_Types(Type* t1, Type* t2, position* pos, Context* c)
{
	assert(t1);
	assert(t2);
	assert(pos);
	assert(c);
	
	if (!Type_Comp(t1, t2))
	{
		fprintf(stderr, "Line %d, character %d: types '", pos->first_line, pos->first_column);
		Print_Type(stderr, t1);
		fprintf(stderr, "' and '");
		Print_Type(stderr, t2);
		fprintf(stderr, "' mismatch.\n");
		c->err = true;
	}
}

static Type* Type_LValue(LValue* lv, Context* c)
{
	assert(lv);
	assert(c);
	if (lv->var)
	{
		symbol* symb = Context_Get(c, lv->v.var.s);
		if (!symb)
		{
			Static_Error(c, &lv->pos, "variable %s is undeclared", lv->v.var.s);
			return NULL;
		}
		return symb->v.t;
	}
	else
	{
		Type* t = Check_Expr(lv->v.e, c);
		if (t->type == TYPE_PTR)
			return t->v.ptr;
		else
		{
			Static_Error(c, &lv->pos, "not a reference");
			return NULL;
		}
	}
}

Type* Check_Expr(Expr* e, Context* c)
{
	assert(e);
	assert(c);
	
	symbol* symb;
	Type*   t1;
	Type*   t2;
	Type*   t3;
	
	switch (e->type)
	{
	case EXPR_INTEGER:
		return &TInt;
	case EXPR_FUN_CALL:
		symb = Context_Get(c, e->v.call.name);
		if (symb)
		{
			e->v.call.id = symb->id;
			Check_ExprList(e->v.call.params, symb->v.f, c);
			return symb->v.f->type;
		}
		else
			Static_Error(c, &e->pos, "function %s is undeclared", e->v.call.name);
	case EXPR_AFF:
		t1 = Check_Expr(e->v.aff.expr, c);
		t2 = Type_LValue(e->v.aff.lv, c);
		Check_Types(t1, t2, &e->pos, c);
		if (e->v.aff.lv->var)
		{
			symb = Context_Get(c, e->v.aff.lv->v.var.s);
			if (symb)
				e->v.aff.lv->v.var.id = symb->id;
			else
				Static_Error(c, &e->v.aff.lv->pos, "variable %s is undeclared", e->v.aff.lv->v.var.s);
		}
		return t1;
	case EXPR_VAR:
	case EXPR_ADDR:
		symb = Context_Get(c, e->v.var.name);
		if (symb)
		{
			e->v.var.id = symb->id;
			return symb->v.t;
		}
		else
			Static_Error(c, &e->pos, "variable %s is undeclared", e->v.var.name);
	case EXPR_NOT:
	case EXPR_LNOT:
	case EXPR_MINUS:
		return Check_Expr(e->v.uni_op, c);
	case EXPR_AND:  case EXPR_OR:  case EXPR_XOR:
	case EXPR_LAND: case EXPR_LOR:
	case EXPR_EQ:   case EXPR_NEQ: case EXPR_LE:  case EXPR_LT:  case EXPR_GE:  case EXPR_GT:
	case EXPR_ADD:  case EXPR_SUB: case EXPR_MUL: case EXPR_DIV: case EXPR_MOD:
		t1 = Check_Expr(e->v.bin_op.left, c);
		t2 = Check_Expr(e->v.bin_op.right, c);
		Check_Types(t1, t2, &e->pos, c);
		return t1;
	case EXPR_IFTE:
		t1 = Check_Expr(e->v.tern_op.op1, c);
		t2 = Check_Expr(e->v.tern_op.op2, c);
		t3 = Check_Expr(e->v.tern_op.op3, c);
		Check_Types(t1, t2, &e->pos, c);
		Check_Types(t1, t3, &e->pos, c);
		return t1;
	case EXPR_DEREF:
		t1 = Check_Expr(e->v.uni_op, c);
		if (t1->type == TYPE_PTR)
			return t1->v.ptr;
		else
			Static_Error(c, &e->pos, "dereferencing non pointer value");
	}
	
	return NULL;
}

void Check_ExprList(ExprList* l, FunDecl* fd, Context* c)
{
	assert(c);
	
	ParamList* p = fd->params;
	while (l && p)
	{
		Type* t = Check_Expr(l->head, c);
		Check_Types(p->head->type, t, &p->head->pos, c);
		p = p->tail;
		l = l->tail;
	}
	if (l)
		Static_Error(c, &fd->pos, "too many arguments to function %s", fd->name);
	if (p && p->head->type->type != TYPE_VOID)
		Static_Error(c, &fd->pos, "too few arguments to function %s", fd->name);
}

void Check_Stmt(Stmt* s, bool needRet, Context* c)
{
	assert(s);
	assert(c);
	
	symbol* symb;
	string  name;
	Type*   t;
	switch (s->type)
	{
	case STMT_NOTHING:
		break;
	case STMT_DECL:
		name = s->v.decl.name;
		symb = Context_Get(c, name);
		if (!Context_CanDeclare(c, name))
		{
			Static_Error(c, &s->pos, "redeclaration of %s", name);
			Static_Error(c, &s->pos, "previous declaration was here: Line %d, character %d", symb->pos->first_line, symb->pos->first_column);
		}
		else
		{
			symb = Context_Declare(c, name);
			symb->isFun      = false;
			symb->isDefined  = false;
			symb->pos        = &s->pos;
			symb->v.t        = s->v.decl.t;
			
			s->v.decl.id = symb->id;
		}
		break;
	case STMT_EXPR:
		Check_Expr(s->v.expr, c);
		break;
	case STMT_RETURN:
		t = Check_Expr(s->v.expr, c);
		Check_Types(c->cur_fun->type, t, &s->pos, c);
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
		Context_BeginScope(c);
		Check_Stmt(s->v.forz.a,    false, c);
		Check_Expr(s->v.forz.b,           c);
		Check_Stmt(s->v.forz.c,    false, c);
		Check_Stmt(s->v.forz.stmt, false, c);
		Context_EndScope(c);
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
		Context_BeginScope(c);
		Check_StmtList(s->v.block, needRet, c);
		Context_EndScope(c);
		break;
	}

	if (needRet && s->type != STMT_BLOCK && s->type != STMT_RETURN && s->type != STMT_IF)
		Static_Error(c, &c->cur_fun->pos, "no return statement at end of non-void function '%s'", c->cur_fun->name);
}

void Check_StmtList(StmtList* l, bool needRet, Context* c)
{
	assert(c);
	
	if (!l && needRet)
		Static_Error(c, &c->cur_fun->pos, "no return statement at end of non-void function '%s'", c->cur_fun->name);
	while (l)
	{
		Check_Stmt(l->head, !l->tail && needRet, c);
		l = l->tail;
	}
}

void Check_Param(Param* p, Context* c)
{
	assert(p);
	assert(c);
	
	string  name = p->name;
	symbol* symb = Context_Get(c, name);
	if (symb)
	{
		Static_Error(c, &p->pos, "redeclaration of %s", name);
		Static_Error(c, &p->pos, "previous declaration was here: Line %d, character %d", symb->pos->first_line, symb->pos->first_column);
	}
	else
	{
		symb = Context_Declare(c, name);
		symb->isDefined  = false;
		symb->isFun      = false;
		symb->pos        = &p->pos;
		symb->v.t        = p->type;
		
		p->id        = symb->id;
	}
}

void Check_ParamList(ParamList* l, Context* c)
{
	assert(c);
	
	while (l)
	{
		Check_Param(l->head, c);
		l = l->tail;
	}
}

void Check_FunDecl(FunDecl* fd, Context* c)
{
	assert(fd);
	assert(c);
	
	string  name = fd->name;
	symbol* symb = Context_Get(c, name);
	if (symb)
	{
		Static_Error(c, &fd->pos, "redeclaration of %s", name);
		Static_Error(c, &fd->pos, "previous declaration was here: Line %d, character %d", symb->pos->first_line, symb->pos->first_column);
	}
	else
	{
		symb = Context_Declare(c, name);
		symb->isFun      = true;
		symb->pos        = &fd->pos;
		symb->v.f        = fd;
		
		if (!strcmp(name, "main"))
			c->main = symb->id;
		
		c->cur_fun = fd;
		fd->id = symb->id;
		Context_BeginScope(c);
		Check_ParamList(fd->params, c);
		Check_Stmt(fd->stmt, fd->type->type != TYPE_VOID, c);
		Context_EndScope(c);
	}
}

void Check_Program(Program* l, Context* c)
{
	assert(c);
	
	Context_BeginScope(c);
	
	symbol* symb;
	
	symb = Context_Declare(c, "malloc");
	symb->isFun  = true;
	symb->v.f    = FunDecl_New(Type_Int(), "malloc", ParamList_New(Param_New(Type_Int(), "size", NULL), NULL), NULL, NULL);
	
	symb = Context_Declare(c, "free");
	symb->isFun  = true;
	symb->v.f    = FunDecl_New(Type_Void(), "free", ParamList_New(Param_New(Type_Ptr(Type_Int()), "ptr", NULL), NULL), NULL, NULL);

	while (l)
	{
		Check_FunDecl(l->head, c);
		l = l->tail;
	}
	//Context_EndScope(c); // TODO
}
