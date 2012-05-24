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

#ifndef STATIC_H
#define STATIC_H

#include "context.h"

Type* Check_Expr     (Expr*,             Context*);
void  Check_ExprList (ExprList*,  Decl*, Context*);
void  Check_Stmt     (Stmt*,      bool,  Context*);
void  Check_StmtList (StmtList*,  bool,  Context*);
void  Check_Param    (Param*,            Context*);
void  Check_ParamList(ParamList*,        Context*);
void  Check_FunDecl  (Decl*,             Context*);
void  Check_VarDecl  (Decl*,             Context*);
void  Check_Struct   (Decl*,             Context*);
void  Check_Decl     (Decl*,             Context*);
void  Check_Program  (Program*,          Context*);

#endif
