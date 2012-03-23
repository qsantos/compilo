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

#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H

#include "ast.h"

void ExprList_Print(ExprList*);
void Expr_Print(Expr*);
void StmtList_Print(StmtList*);
void Stmt_Print(Stmt*);
void Param_Print(Param*);
void ParamList_Print(ParamList*);
void FunDecl_Print(FunDecl*);
void Program_Print(Program*);

#endif
