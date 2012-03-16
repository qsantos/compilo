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

typedef struct
{
	bool isDeclared; // déjà déclaré ?
	bool isDefined;  // a-t'il déjà été initialisé ?
	bool isFun;      // variable "normale" ou fonction ?
	union
	{
		Type*    t;
		FunDecl* f;
	} v;
	position* pos;
} symbol;

typedef struct
{
	bool       err; // une erreur a déjà eu lieu ?
	HashTable* ht;
	symbol*    st;
	u32stack*  defined; // cf commentaires dans le .c
	u32stack*  forget;
	u32        depth;
} context;

context* Context_New(u32);
void Context_Delete(context*);

void Check_Expr(Expr*, context*);
void Check_ExprList(ExprList*, context*);
void Check_Stmt(Stmt*, context*);
void Check_StmtList(StmtList*, context*);
void Check_Param(Param*, context*);
void Check_ParamList(ParamList*, context*);
void Check_FunDecl(FunDecl*, context*);
void Check_Program(Program*, context*);

#endif
