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

#include "static.h"

#include <assert.h>

/* Static analysis */

void Check_Expr(Expr* e, Context* c)
{
	assert(e);
	assert(c);
	
	symbol* symb;
	string  name;
	switch (e->type)
	{
	case EXPR_FUN_CALL:
		name         = e->v.call.name;
		symb         = Context_Get(c, name);
		e->v.call.id = symb->id;
		if (!symb)
		{
			Static_Error(c, &e->pos, "function %s is undeclared", name);
		}
		Check_ExprList(e->v.call.params, c);
		if (!c->err)
			Check_TypeParams(symb->v.f, e, c);
		break;
	case EXPR_AFF:
		name        = e->v.aff.name;
		symb        = Context_Get(c, name);
		e->v.aff.id = symb->id;
		if (!symb)
		{
			Static_Error(c, &e->pos, "variable %s is undeclared", name);
		}
		Check_Expr(e->v.aff.expr, c);
		if (!c->err)
			Check_TypeExpr(symb->v.t, e->v.aff.expr, c);
		break;
	case EXPR_VAR:
		name        = e->v.var.name;
		symb        = Context_Get(c, name);
		e->v.var.id = symb->id;
		if (!symb)
		{
			Static_Error(c, &e->pos, "variable %s is undeclared", name);
		}
		break;
	case EXPR_NEG:
	case EXPR_NOT:
	case EXPR_MINUS:
		Check_Expr(e->v.uni_op, c);
		break;
	case EXPR_AND:  case EXPR_OR:  case EXPR_XOR:
	case EXPR_LAND: case EXPR_LOR:
	case EXPR_EQ:   case EXPR_NEQ: case EXPR_LE:  case EXPR_LT:  case EXPR_GE:  case EXPR_GT:
	case EXPR_ADD:  case EXPR_SUB: case EXPR_MUL: case EXPR_DIV: case EXPR_MOD:
		Check_Expr(e->v.bin_op.left,  c);
		Check_Expr(e->v.bin_op.right, c);
		break;
	case EXPR_IFTE:
		Check_Expr(e->v.tern_op.op1, c);
		Check_Expr(e->v.tern_op.op2, c);
		Check_Expr(e->v.tern_op.op3, c);
		break;
	case EXPR_DEREF:
	case EXPR_ADDR:
		Type_Expr(e->v.uni_op, c);
		Check_Expr(e, c);
		break;
	default:
		/* No problem... */
		break;
	}
}

void Check_ExprList(ExprList* l, Context* c)
{
	while (l)
	{
		Check_Expr(l->head, c);
		l = l->tail;
	}
}

void Check_Stmt(Stmt* s, bool needRet, Context* c)
{
	assert(s);
	assert(c);
	
	symbol* symb;
	string  name;
	switch (s->type)
	{
	case STMT_DECL:
		name         = s->v.decl.name;
		symb         = Context_Get(c, name);
		if (!Context_CanDeclare(c, name))
		{
			Static_Error(c, &s->v.decl.pos, "redeclaration of %s", name);
			Static_Error(c, &s->v.decl.pos, "previous declaration was here: Line %d, character %d", symb->pos->first_line, symb->pos->first_column);
		}
		else
		{
			symb = Context_Declare(c, name);
			symb->isFun      = false;
			symb->pos        = &s->v.decl.pos;
			symb->v.t        = s->v.decl.t;
			
			s->v.decl.id = symb->id;
			if (s->v.decl.val)
			{
				Check_Expr(s->v.decl.val, c);
				symb->isDefined = true;
				Check_TypeExpr(s->v.decl.t, s->v.decl.val, c);
			}
			else
				symb->isDefined = false;
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
		Check_Expr(s->v.forz.b,           c);
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
		Context_BeginScope(c);
		Check_StmtList(s->v.block, needRet, c);
		Context_EndScope(c);
		break;
	default:
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
	       
		c->cur_fun = fd;
		fd->id = symb->id;
		Context_BeginScope(c);
		Check_ParamList(fd->params, c);
		Check_Stmt(fd->stmt, !Type_Comp(fd->type, &TVoid), c);
		Context_EndScope(c);
	}
}

void Check_Program(Program* l, Context* c)
{
	assert(c);
	
	Context_BeginScope(c);
	while (l)
	{
		Check_FunDecl(l->head, c);
		l = l->tail;
	}
	Context_EndScope(c);
}

/* Typage */

Type* Type_Expr(Expr* e, Context* c)
{
	assert(e);
	assert(c);
	
	symbol* symb;
	Type*   t;
	switch (e->type)
	{
	case EXPR_INTEGER:
		return &TInt;
	case EXPR_FUN_CALL:
		symb = Context_Get(c, e->v.call.name);
		return symb->v.f->type;
	case EXPR_AFF:
		symb = Context_Get(c, e->v.aff.name);
		t = symb->v.t;
		Check_Types(t, Type_Expr(e->v.aff.expr, c), &e->pos, c);
		return t;
	case EXPR_VAR:
		symb = Context_Get(c, e->v.var.name);
		return symb->v.t;
	case EXPR_NEG:
	case EXPR_NOT:
	case EXPR_MINUS:
		return Type_Expr(e->v.uni_op, c);
	case EXPR_AND:  case EXPR_OR:  case EXPR_XOR:
	case EXPR_LAND: case EXPR_LOR:
	case EXPR_EQ:   case EXPR_NEQ: case EXPR_LE:  case EXPR_LT:  case EXPR_GE:  case EXPR_GT:
	case EXPR_ADD:  case EXPR_SUB: case EXPR_MUL: case EXPR_DIV: case EXPR_MOD:
		t = Type_Expr(e->v.bin_op.left, c);
		Check_Types(t, Type_Expr(e->v.bin_op.right, c), &e->pos, c);
		return t;
	case EXPR_IFTE:
		t = Type_Expr(e->v.tern_op.op1, c);
		t = Type_Expr(e->v.tern_op.op2, c);
		Check_Types(t, Type_Expr(e->v.tern_op.op3, c), &e->pos, c);
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

void Check_Types(Type* t1, Type* t2, position* pos, Context* c)
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

void Check_TypeExpr(Type* t, Expr* e, Context* c)
{
	assert(t);
	assert(e);
	assert(c);
	
	Check_Types(t, Type_Expr(e, c), &e->pos, c);
}

void Check_TypeParams(FunDecl* fd, Expr* e, Context* c)
{
	assert(fd);
	assert(e);
	assert(c);
	
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
