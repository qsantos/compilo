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

#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* EXPRESSIONS */

Expr* Expr_Integer(s32 i)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = EXPR_INTEGER;
  expr->v.i = i;
  return expr;
}

Expr* Expr_Fun_Call(string name, ExprList* params)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = EXPR_FUN_CALL;
  expr->v.call.name   = name;
  expr->v.call.params = params;
  return expr;
}

Expr* Expr_Aff(string name, Expr* e)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = EXPR_AFF;
  expr->v.aff.name = name;
  expr->v.aff.expr = e;
  return expr;
}

Expr* Expr_Var(string name)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = EXPR_VAR;
  expr->v.var = name;
  return expr;
}

Expr* Expr_Neg(Expr* e)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = EXPR_NEG;
  expr->v.uni_op = e;
  return expr;
}

static Expr* Expr_binop(int type, Expr* l, Expr* r)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = type;
  expr->v.bin_op.left = l;
  expr->v.bin_op.right = r;
  return expr;
}

Expr* Expr_Eq (Expr* l, Expr* r) { return Expr_binop(EXPR_EQ,  l, r); }
Expr* Expr_Neq(Expr* l, Expr* r) { return Expr_binop(EXPR_NEQ, l, r); }
Expr* Expr_Le (Expr* l, Expr* r) { return Expr_binop(EXPR_LE,  l, r); }
Expr* Expr_Lt (Expr* l, Expr* r) { return Expr_binop(EXPR_LT,  l, r); }
Expr* Expr_Ge (Expr* l, Expr* r) { return Expr_binop(EXPR_GE,  l, r); }
Expr* Expr_Gt (Expr* l, Expr* r) { return Expr_binop(EXPR_GT,  l, r); }
Expr* Expr_Add(Expr* l, Expr* r) { return Expr_binop(EXPR_ADD, l, r); }
Expr* Expr_Sub(Expr* l, Expr* r) { return Expr_binop(EXPR_SUB, l, r); }
Expr* Expr_Mul(Expr* l, Expr* r) { return Expr_binop(EXPR_MUL, l, r); }
Expr* Expr_Div(Expr* l, Expr* r) { return Expr_binop(EXPR_DIV, l, r); }
Expr* Expr_Modulo(Expr* l, Expr* r) { return Expr_binop(EXPR_MODULO,  l, r); }

Expr* Expr_Minus(Expr* op)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = EXPR_MINUS;
  expr->v.uni_op = op;
  return expr;
}

Expr* Expr_Ifte(Expr* c, Expr* a, Expr* b)
{
  Expr* expr = (Expr*) malloc(sizeof(Expr));
  assert(expr);
  expr->type = EXPR_IFTE;
  expr->v.tern_op.op1 = c;
  expr->v.tern_op.op2 = a;
  expr->v.tern_op.op3 = b;
  return expr;
}


ExprList* ExprList_New(Expr* head, ExprList* tail)
{
  ExprList* list = (ExprList*) malloc(sizeof(ExprList));
  assert(list);
  list->head = head;
  list->tail = tail;
  return list;  
}

void Expr_Delete(Expr* e)
{
  if (e)
    {
      switch (e->type)
	{
	case EXPR_FUN_CALL:
	  free(e->v.call.name);
	  ExprList_Delete(e->v.call.params);
	  break;
	case EXPR_VAR:
	  free(e->v.var);
	  break;
	case EXPR_ADD:
	case EXPR_SUB:
	case EXPR_MUL:
	case EXPR_DIV:
	  Expr_Delete(e->v.bin_op.left);
	  Expr_Delete(e->v.bin_op.right);
	  break;
	case EXPR_MINUS:
	  Expr_Delete(e->v.uni_op);
	  break;
	default:
	  break;
	}
      free(e);
    }
}

void ExprList_Delete(ExprList* l)
{
  if (l)
    {
      Expr_Delete(l->head);
      ExprList_Delete(l->tail);
      free(l);
    }
}

/* TYPES */

static Type TVoid  = { TYPE_VOID, { .ptr = NULL } };
static Type TChar  = { TYPE_CHAR, { .ptr = NULL } };
static Type TInt   = { TYPE_INT,  { .ptr = NULL } };

Type* Type_Void(void)
{
  return &TVoid;
}

Type* Type_Char(void)
{
  return &TChar;
}

Type* Type_Int(void)
{
  return &TInt;
}

Type* Type_Ptr(Type* ptr)
{
  Type* t = (Type*) malloc(sizeof(Type));
  assert(t);
  t->type = TYPE_PTR;
  t->v.ptr = ptr;
  return t;
}

void Type_Delete(Type* t)
{
  (void) t;
}

/* STATEMENTS */

static Stmt SNothing = { STMT_NOTHING, { .expr = NULL } };

Stmt* Stmt_Nothing(void)
{
  return &SNothing;
}

Stmt* Stmt_Decl(Type* t, string name, Expr* init)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_DECL;
  s->v.decl.t    = t;
  s->v.decl.name = name;
  s->v.decl.val  = init;
  return s;
}

Stmt* Stmt_Aff(string name, Expr* val)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_AFF;
  s->v.aff.name = name;
  s->v.aff.val  = val;
  return s;
}

Stmt* Stmt_Expr(Expr* expr)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_EXPR;
  s->v.expr = expr;
  return s;
}

Stmt* Stmt_While(Expr* cond, Stmt* stmt)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_WHILE;
  s->v.whilez.cond = cond;
  s->v.whilez.stmt = stmt;
  return s;
}

Stmt* Stmt_Do(Stmt* stmt, Expr* cond)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_DO;
  s->v.doz.stmt = stmt;
  s->v.doz.cond = cond;
  return s;
}
Stmt* Stmt_For(Stmt* a, Stmt* b, Stmt* c, Stmt* stmt)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_FOR;
  s->v.forz.a = a;
  s->v.forz.b = b;
  s->v.forz.c = c;
  s->v.forz.stmt = stmt;
  return s;
}

Stmt* Stmt_If(Expr* cond , Stmt* iftrue, Stmt* iffalse)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_IF;
  s->v.ifz.cond   = cond;
  s->v.ifz.iftrue = iftrue;
  s->v.ifz.iffalse = iffalse;
  return s;
}

Stmt* Stmt_Return(Expr* expr)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_RETURN;
  s->v.expr = expr;
  return s;
}

Stmt* Stmt_Block(StmtList* block)
{
  Stmt* s = (Stmt*) malloc(sizeof(Stmt));
  assert(s);
  s->type = STMT_BLOCK;
  s->v.block = block;
  return s;
}

StmtList* StmtList_New(Stmt* head, StmtList* tail)
{
  StmtList* list = (StmtList*) malloc(sizeof(StmtList));
  assert(list);
  list->head = head;
  list->tail = tail;
  return list;  
}

void Stmt_Delete(Stmt* s)
{
  if (s)
    {
      switch (s->type)
	{
	case STMT_DECL:
	  Type_Delete(s->v.decl.t);
	  free(s->v.decl.name);
	  Expr_Delete(s->v.decl.val);
	  break;
	case STMT_AFF:
	  free(s->v.aff.name);
	  Expr_Delete(s->v.aff.val);
	  break;
	case STMT_EXPR:
	case STMT_RETURN:
	  Expr_Delete(s->v.expr);
	  break;
	case STMT_WHILE:
	  Expr_Delete(s->v.whilez.cond);
	  Stmt_Delete(s->v.whilez.stmt);
	  break;
	case STMT_IF:
	  Expr_Delete(s->v.ifz.cond);
	  Stmt_Delete(s->v.ifz.iftrue);
	  Stmt_Delete(s->v.ifz.iffalse);
	  break;
	case STMT_BLOCK:
	  StmtList_Delete(s->v.block);
	  break;
	default:
	  break;
	}
      if (s->type != STMT_NOTHING)
	free(s);
    }
}

void StmtList_Delete(StmtList* l)
{
  if (l)
    {
      Stmt_Delete(l->head);
      StmtList_Delete(l->tail);
      free(l);
    }
}

/* FUNCTION DECLARATIONS */

Param* Param_New(Type* t, string name)
{
  Param* p = (Param*) malloc(sizeof(Param));
  assert(p);
  p->type = t;
  p->name = name;
  return p;
}

ParamList* ParamList_New(Param* head, ParamList* tail)
{
  ParamList* list = (ParamList*) malloc(sizeof(ParamList));
  assert(list);
  list->head = head;
  list->tail = tail;
  return list;  
}

ParamList* ParamList_Void(void)
{
  return ParamList_New(Param_New(Type_Void(), ""), NULL);
}

FunDecl* FunDecl_New(Type* ret, string name, ParamList* params, Stmt* body)
{
  FunDecl* f = (FunDecl*) malloc(sizeof(FunDecl));
  assert(f);
  f->type   = ret;
  f->name   = name;
  f->params = params;
  f->stmt   = body;
  return f;
}

Program* Program_New(FunDecl* head, Program* tail)
{
  Program* p = (Program*) malloc(sizeof(Program));
  assert(p);
  p->head = head;
  p->tail = tail;
  return p;  
}

void Param_Delete(Param* p)
{
  if (p)
    {
      Type_Delete(p->type);
      if (*p->name) free(p->name);
      free(p);
    }
}

void ParamList_Delete(ParamList* l)
{
  if (l)
    {
      Param_Delete(l->head);
      ParamList_Delete(l->tail);
      free(l);
    }
}

void FunDecl_Delete(FunDecl* f)
{
  if (f)
    {
      Type_Delete(f->type);
      free(f->name);
      ParamList_Delete(f->params);
      Stmt_Delete(f->stmt);
      free(f);
    }
}

void Program_Delete(Program* p)
{
  if (p)
    {
      FunDecl_Delete(p->head);
      Program_Delete(p->tail);
      free(p);
    }
}
