/*\
 *       \\          A Tiny C to MIPS Compiler                     //
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
#include "latex.h"

#include <stdio.h>

static u8 depth = 3;
static void indent()
{
	for (u8 i = 0; i < depth; i++)
		putchar('\t');
}
static void beginNode(const char* name)
{
	indent();printf("child\n");
	indent();printf("{\n");
	depth++;
	indent();printf("node{%s}\n", name);
}
static void beginNodeInt(s32 s)
{
	indent();printf("child\n");
	indent();printf("{\n");
	depth++;
	indent();printf("node{%ld}\n", s);
}
static void endNode()
{
	indent();printf("edge from parent node[below]{}\n");
	depth--;
	indent();printf("}\n");
}

static void printName(const char* str);
static void printExprList(ExprList* l);
static void printExpr(Expr* e);
static void printType(Type* t);
static void printStmtList(StmtList* l);
static void printStmt(Stmt* s);
static void printParam(Param* p);
static void printParams(ParamList* l);
static void printFunDecl(Decl* f);

static void printName(const char* str)
{
	beginNode(str);
	endNode();
}

static void printLValue(LValue* l)
{
	if (l->var)
		printName(l->v.var.s);
	else
	{
		printf("*");
		printExpr(l->v.e);
	}
}

static void printExprList(ExprList* l)
{
	if (!l)
	{
		printName("NULL");
		return;
	}
	do
	{
		printExpr(l->head);
	}
	while ((l = l->tail));
}

#define PRINT_BINOP(CODE, NAME)			\
	case EXPR_##CODE:			\
	beginNode(#NAME);			\
	printExpr(e->v.bin_op.left);		\
	printExpr(e->v.bin_op.right);		\
	endNode();				\
	break;
static void printExpr(Expr* e)
{
	if(!e){printName("NULL");return;}
	switch(e->type)
	{
	case EXPR_INTEGER:
		beginNodeInt(e->v.i);
		endNode();
		break;
	case EXPR_FUN_CALL:
		beginNode("FunCall");
		printName(e->v.call.name);
		printExprList(e->v.call.params);
		endNode();
		break;
	case EXPR_AFF:
		beginNode("Aff");
		printLValue(e->v.aff.lv);
		printExpr(e->v.aff.expr);
		endNode();
		break;
	case EXPR_VAR:
		beginNode("Var");
		printName(e->v.var.name);
		endNode();
		break;
		PRINT_BINOP(EQ,   Eq  );
		PRINT_BINOP(NEQ,  Neq );
		PRINT_BINOP(LE,   Le  );
		PRINT_BINOP(LT,   Lt  );
		PRINT_BINOP(GE,   Ge  );
		PRINT_BINOP(GT,   Gt  );
		PRINT_BINOP(ADD,  Add );
		PRINT_BINOP(SUB,  Sub );
		PRINT_BINOP(MUL,  Mul );
		PRINT_BINOP(DIV,  Div );
		PRINT_BINOP(MOD,  Mod );
		PRINT_BINOP(NOT,  Not );
		PRINT_BINOP(AND,  And );
		PRINT_BINOP(OR,   Or  );
		PRINT_BINOP(XOR,  Xor );
		PRINT_BINOP(LNOT, LNot);
		PRINT_BINOP(LAND, LAnd);
		PRINT_BINOP(LOR,  LOr );
	case EXPR_MINUS:
		beginNode("Minus");
		printExpr(e->v.uni_op);
		endNode();
		break;
	case EXPR_DEREF:
		beginNode("Deref");
		printExpr(e->v.uni_op);
		endNode();
		break;
	case EXPR_ADDR:
		beginNode("Addr");
		printExpr(e->v.uni_op);
		endNode();
		break;
	case EXPR_IFTE:
		beginNode("Ifte");
		printExpr(e->v.tern_op.op1);
		printExpr(e->v.tern_op.op2);
		printExpr(e->v.tern_op.op3);
		endNode();
	}
}

static void printType(Type* t)
{
	if(!t){printName("NULL");return;}
	switch (t->type)
	{
	case TYPE_VOID:
		printName("void");
		break;
	case TYPE_CHAR:
		printName("char");
		break;
	case TYPE_INT:
		printName("int");
		break;
	case TYPE_PTR:
		printType(t->v.ptr);
		printName("*");
		break;
	}
}

static void printStmtList(StmtList* l)
{
	do
	{
		printStmt(l->head);
	}
	while ((l = l->tail));
}

static void printStmt(Stmt* s)
{
	switch (s->type)
	{
	case STMT_NOTHING:
		beginNode("Nothing");
		endNode();
		break;
	case STMT_DECL:
		beginNode("Decl");
		printType(s->v.decl.t);
		printName(s->v.decl.name);
		endNode();
		break;
	case STMT_EXPR:
		printExpr(s->v.expr);
		break;
	case STMT_WHILE:
		beginNode("While");
		printExpr(s->v.whilez.cond);
		printStmt(s->v.whilez.stmt);
		endNode();
		break;
	case STMT_DO:
		beginNode("Do");
		printStmt(s->v.doz.stmt);
		printExpr(s->v.doz.cond);
		endNode();
	case STMT_FOR:
		beginNode("For");
		printStmt(s->v.forz.a);
		printExpr(s->v.forz.b);
		printStmt(s->v.forz.c);
		printStmt(s->v.forz.stmt);
		endNode();
		break;
	case STMT_IF:
		beginNode("If");
		printExpr(s->v.ifz.cond);
		printStmt(s->v.ifz.iftrue);
		if (s->v.ifz.iffalse)
			printStmt(s->v.ifz.iffalse);
		endNode();
		break;
	case STMT_RETURN:
		beginNode("Return");
		printExpr(s->v.expr);
		endNode();
		break;
	case STMT_BLOCK:
		beginNode("Block");
		printStmtList(s->v.block);
		endNode();
		break;
	}
}

static void printParam(Param* p)
{
	if(!p){printName("NULL");return;}
	beginNode("Param");
	printType(p->type);
	printName(p->name);
	endNode();
}

static void printParams(ParamList* l)
{
	if(!l){printName("NULL");return;}
	beginNode("ParamList");
	do
	{
		printParam(l->head);
	}
	while ((l = l->tail));
	endNode();
}

static void printFunDecl(Decl* f)
{
	if(!f){printName("NULL");return;}
	beginNode("FunDecl");
	printType(f->v.fun.type);
	printName(f->v.fun.name);
	printParams(f->v.fun.params);
	printStmt(f->v.fun.stmt);
	endNode();
}

void Program_Latex(Program* p)
{
	if (!p)return;
  
	printf
		(
			"\\documentclass[landscape]{article}\n"
			"\\usepackage{tikz}\n"
			"\\usetikzlibrary{trees}\n"
			"\\begin{document}\n"
			"\t\\begin{center}\n"
			"\t\t\\tikzstyle{level 1}=[level distance=3.5cm, sibling distance=2cm]\n"
			"\t\t\\tikzstyle{level 2}=[level distance=3.5cm, sibling distance=2cm]\n"
			"\t\t\\begin{tikzpicture}\n"
			);
  
	printf("\t\t\t\\node{Program}\n");
  
	do
	{
		printFunDecl(p->head);
	}
	while ((p = p->tail));
  
	printf("\t\t\t;\n");
	printf("\t\t\\end{tikzpicture}\n");
	printf("\t\\end{center}\n");
	printf("\\end{document}\n");
}
