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

#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "printer.h"
#include "latex.h"
#include "static.h"
#include "flow.h"
#include "mips.h"

extern int yyparse(void);

Program* current_prog = NULL;
int main(int argc, char** argv)
{
	u32 res = yyparse();
	if (res)
		return res;
	
	if (argc > 1)
	{
		if (!strcmp(argv[1], "--latex"))
		{
			Program_Latex(current_prog);
			return 0;
		}
		if (!strcmp(argv[1], "--ast"))
		{
			Print_Program(current_prog);
			return 0;
		}
	}
	
	Context* c = Context_New(32768);
	Check_Program(current_prog, c);
	if (c->err)
	{
		Context_Delete(c);
		Program_Delete(current_prog);
		return 1;
	}
	ASM* a = ASM_New(c);
	ASM_GenProgram(a, c, current_prog);
	
	
	if (argc > 1)
	{
		if (!strcmp(argv[1], "--asm"))
		{
			Print_ASM(a);
			return 0;
		}
	}
	
	ASM_toMIPS(a, c);
	
	ASM_Delete(a);
	Context_Delete(c);
	Program_Delete(current_prog);
	return 0;
}
