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

#ifndef AST_H
#define AST_H

#include <stdio.h>

#include "types.h"

typedef struct
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} position;

/* Expressions */
typedef struct Expr Expr;
typedef struct ExprList
{
	Expr*            head;
	struct ExprList* tail;
} ExprList;
struct Expr
{
	enum
	{
		EXPR_INTEGER,
		EXPR_FUN_CALL,
		EXPR_AFF,
		EXPR_VAR,
		EXPR_NEG, EXPR_AND,  EXPR_OR,  EXPR_XOR,
		EXPR_NOT, EXPR_LAND, EXPR_LOR,
		EXPR_EQ,  EXPR_NEQ,  EXPR_LE,  EXPR_LT,  EXPR_GE,  EXPR_GT,
		EXPR_ADD, EXPR_SUB,  EXPR_MUL, EXPR_DIV, EXPR_MOD,
		EXPR_MINUS,
		EXPR_IFTE,
		EXPR_DEREF,
		EXPR_ADDR,
	} type;
	union
	{
		s32 i;
		struct { string name; u32 id; ExprList* params; } call;
		struct { string name; u32 id; Expr* expr; } aff;
		struct { string name; u32 id; } var;
		struct { struct Expr* left, * right; } bin_op;
		struct { struct Expr* op1, * op2, * op3; } tern_op;
		struct Expr* uni_op;
	} v;
	position pos;
};
/* Constructors */
Expr* Expr_Integer(s32, position*);
Expr* Expr_Fun_Call(string, ExprList*, position*);
Expr* Expr_Aff (string, Expr*, position*);
Expr* Expr_Var (string, position*);
Expr* Expr_Neg (Expr*, position*);
Expr* Expr_Not (Expr*, position*);
Expr* Expr_And (Expr*, Expr*, position*);
Expr* Expr_Or  (Expr*, Expr*, position*);
Expr* Expr_Xor (Expr*, Expr*, position*);
Expr* Expr_Land(Expr*, Expr*, position*);
Expr* Expr_Lor (Expr*, Expr*, position*);
Expr* Expr_Eq  (Expr*, Expr*, position*);
Expr* Expr_Neq (Expr*, Expr*, position*);
Expr* Expr_Le  (Expr*, Expr*, position*);
Expr* Expr_Lt  (Expr*, Expr*, position*);
Expr* Expr_Ge  (Expr*, Expr*, position*);
Expr* Expr_Gt  (Expr*, Expr*, position*);
Expr* Expr_Add (Expr*, Expr*, position*);
Expr* Expr_Sub (Expr*, Expr*, position*);
Expr* Expr_Mul (Expr*, Expr*, position*);
Expr* Expr_Div (Expr*, Expr*, position*);
Expr* Expr_Mod (Expr*, Expr*, position*);
Expr* Expr_Minus(Expr*, position*);
Expr* Expr_Deref(Expr*, position*);
Expr* Expr_Addr(Expr*, position*);
Expr* Expr_Ifte(Expr*, Expr*, Expr*, position*);
ExprList* ExprList_New(Expr*, ExprList*);
/* Destructors */
void Expr_Delete(Expr*);
void ExprList_Delete(ExprList*);



/* Types  */
typedef struct Type
{
	enum
	{
		TYPE_VOID,
		TYPE_CHAR,
		TYPE_INT,
		TYPE_PTR,
	} type;
	union
	{
		struct Type* ptr;
	} v;
} Type;
extern Type TVoid;
extern Type TChar;
extern Type TInt;
/* Contructors */
Type* Type_Void(void);
Type* Type_Char(void);
Type* Type_Int(void);
Type* Type_Ptr(Type*);
/* Destructors */
void Type_Delete(Type*);
/* Utils */
bool Type_Comp(Type*, Type*);
void Type_Print(FILE*, Type*);



/* Statements */
typedef struct Stmt Stmt;
typedef struct StmtList
{
	Stmt*            head;
	struct StmtList* tail;
} StmtList;
struct Stmt
{
	enum
	{
		STMT_NOTHING,
		STMT_DECL,
		STMT_EXPR,
		STMT_WHILE,
		STMT_DO,
		STMT_FOR,
		STMT_IF,
		STMT_RETURN,
		STMT_BLOCK,
	} type;
	union
	{
		struct { Type* t; string name; Expr* val; position pos; } decl;
		Expr* expr;
		struct { Expr* cond; Stmt* stmt; } whilez;
		struct { Stmt* stmt; Expr* cond; } doz;
		struct { Stmt* a; Stmt* b; Stmt* c; Stmt* stmt; } forz;
		struct { Expr* cond; Stmt* iftrue, * iffalse; } ifz;
		StmtList* block;
	} v;
	position pos;
};
/* Constructors */
Stmt* Stmt_Nothing(void);
Stmt* Stmt_Decl(Type*, string, Expr*, position* pos);
Stmt* Stmt_Expr(Expr*);
Stmt* Stmt_While(Expr*, Stmt*);
Stmt* Stmt_Do(Stmt*, Expr*);
Stmt* Stmt_For(Stmt*, Stmt*, Stmt*, Stmt*);
Stmt* Stmt_If(Expr*, Stmt*, Stmt*);
Stmt* Stmt_Return(Expr*);
Stmt* Stmt_Block(StmtList*);
StmtList* StmtList_New(Stmt*, StmtList*);
/* Destructors */
void Stmt_Delete(Stmt*);
void StmtList_Delete(StmtList*);



/* Function declarations */
typedef struct
{
	Type*    type;
	string   name;
	position pos;
} Param;
typedef struct ParamList
{
	Param*            head;
	struct ParamList* tail;
} ParamList;
typedef struct
{
	Type*      type;
	string     name;
	u32        id;
	ParamList* params;
	Stmt*      stmt;
	position   pos;
} FunDecl;
typedef struct Program
{
	FunDecl*        head;
	struct Program* tail;
} Program;
/* Constructors */
Param* Param_New(Type*, string, position*);
ParamList* ParamList_New(Param*, ParamList*);
ParamList* ParamList_Void(void);
FunDecl* FunDecl_New(Type*, string, ParamList*, Stmt*, position*);
Program* Program_New(FunDecl*, Program*);
/* Destructors */
void Param_Delete(Param*);
void ParamList_Delete(ParamList*);
void FunDecl_Delete(FunDecl*);
void Program_Delete(Program*);

#endif
