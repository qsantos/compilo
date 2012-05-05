/*\
 *       \\          A Tiny C to SPIM Compiler                     //
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

#ifndef FLOW_H
#define FLOW_H

#include "asm.h"
#include "context.h"

typedef struct
{
	u32stack* def;
	u32stack* use;
	
	Set* in;
	Set* out;
	
	s32  mov;
	s32  jmp;
} Salmon;

typedef struct
{
	Salmon* s;
	u32     ns;
	u32     nr;
} Flow;

Flow* Flow_New     (u32, u32);
void  Flow_Delete  (Flow*);
Flow* Flow_Build   (ASM*, u32, u32, Context*);
void  Flow_Vivacity(Flow*);

#endif
