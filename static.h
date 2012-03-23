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

#ifndef STATIC_H
#define STATIC_H

#include "types.h"
#include "ast.h"
#include "u32stack.h"
#include "hash.h"

/* ABOUT THE TABLE OF SYMBOLS
 * When a symbol is defined: pushed on defined, top of forget incremented
 * When a block is opened: 0 is pushed on forget
 * When a block is closed: 'forget' symbols are undeclared and forget is poped
 */

typedef struct
{
	bool isDeclared; // actually exists
	bool isFun;      // variable or function
	bool isDefined;  // if function, has it been defined ? if variable, has it been initialized ?
	union
	{
		Type*    t;
		FunDecl* f;
	} v;
	position* pos;
} symbol;

typedef struct
{
	bool       err;     // true when an error occured
	HashTable* ht;      // transition from string to u32
	symbol*    st;      // information about symbols
	u32stack*  defined; // list of the symbols of the current scope
	u32stack*  forget;  // number of symbols in the current scope
	FunDecl*   cur_fun; // current function (if needed)
} context;

/* CONTEXT ALTERATION */
context* Context_New(u32);                // construtcor
void Context_Delete    (context*);        // destructor
void Context_BeginBlock(context*);        // enters a new scope
void Context_Define    (context*, u32);   // define a symbol for the current scope
void Context_EndBlock  (context*);        // exists the last scope
void Static_Error(context*, position*, cstring, ...);

/* TYPE COMPARISON */
void Type_Check(Type*, Type*, position*, context*);
Type* Type_Expr(Expr*, context*);

/* SYMBOLS DECLARATION, DEFINITION AND TYPE CHECK */
void Check_Expr     (Expr*,            context*);
void Check_ExprList (ExprList*,        context*);
void Check_Stmt     (Stmt*,      bool, context*);
void Check_StmtList (StmtList*,  bool, context*);
void Check_Param    (Param*,           context*);
void Check_ParamList(ParamList*,       context*);
void Check_FunDecl  (FunDecl*,         context*);
void Check_Program  (Program*,         context*);

void Check_TypeExpr  (Type*,    Expr*, context*);
void Check_TypeParams(FunDecl*, Expr*, context*);

#endif
