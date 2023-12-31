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

#ifndef STACK_H
#define STACK_H

#include "types.h"

typedef struct u32stack u32stack;

struct u32stack
{
	u32       head;
	u32stack* tail;
};

u32stack* u32stack_RCopy (u32stack*);
void      u32stack_Delete(u32stack**);
void      u32stack_Push  (u32stack**, u32);
u32       u32stack_Pop   (u32stack**);
void      u32stack_Print (u32stack*);

#endif
