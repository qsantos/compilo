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

#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H

#include "ast.h"
#include "asm.h"

void Print_LValue   (LValue*);
void Print_ExprList (ExprList*);
void Print_Expr     (Expr*);
void Print_StmtList (StmtList*);
void Print_Stmt     (Stmt*);
void Print_Param    (Param*);
void Print_ParamList(ParamList*);
void Print_FunDecl  (FunDecl*);
void Print_Program  (Program*);

void Print_ASM      (ASM*);

#endif
