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

#ifndef CONTEXT_H
#define CONTEXT_H

#include "types.h"
#include "ast.h"
#include "u32stack.h"
#include "hash.h"

/* ABOUT THE SYMBOLS
 * Basically, a symbol is a string.
 * The HashTable provide an integer (local) identifier associated.
 * Depending on the context, the same string can have several meanings.
 * Thus, a transition is made from local to global identifiers.
 */
/* ABOUT THE TABLE OF SYMBOLS
 * When a symbol is declared:
   - local identifier pushed on defined;
   - top of forget incremented;
   - global identifier is pushed on the correspondance table for the local identifier.
 * When a block is opened:
   - 0 is pushed on forget
 * When a block is closed:
   - 'forget' symbols are undeclared (defined and correspondance table are poped)
   - forget is poped;
 */

typedef struct
{
	bool isFun;      // variable or function
	bool isDefined;  // if function, has it been defined ? if variable, has it been initialized ?
	union
	{
		Type*    t;
		FunDecl* f;
	} v;
	position* pos;
	u32       depth; // depth at which it has been declared
} symbol;

typedef struct
{
	bool       err;     // true when an error occured
	HashTable* ht;      // transition from string to u32
	u32stack** l2g;     // local to global transition table (table of stacks);
	symbol*    st;      // information about symbols
	u32stack*  defined; // list of the symbols of the current scope
	u32stack*  forget;  // number of symbols in the current scope
	FunDecl*   cur_fun; // current function (if needed)
	u32        depth;   // helps to know if one can redeclare a variable
} context;

context* Context_New(u32);
void Context_Delete    (context*);
void Context_BeginScope(context*);
void Context_EndScope  (context*);
symbol* Context_Declare(context*, cstring);
symbol* Context_Get    (context*, cstring);
bool Context_CanDeclare(context*, cstring);
void Static_Error(context*, position*, cstring, ...);

#endif