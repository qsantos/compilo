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

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "lexer.h"
#include "printer.h"
#include "latex.h"
#include "static.h"

extern int yyparse(void);

Program* current_prog = NULL;
int main(int argc, char** argv)
{
  yyparse();

  if (argc > 1 && !strcmp(argv[1], "--latex"))
    Program_Latex(current_prog);
  else
    Program_Print(current_prog);

  context* c = Context_New(32000);

  Check_Program(current_prog, c);

  //Context_Delete(c);

  Program_Delete(current_prog);

  return 0;
}
