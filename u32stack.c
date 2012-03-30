/*\
 *       \\          A Tiny C to SPIM Compiler                     //
 *        \\_        Copyright (C) 2012 Thomas  REGOIRE          _//
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

#include "u32stack.h"

#include <stdlib.h>
#include <assert.h> 

void u32stack_delete(u32stack** s)
{
	while (*s)
		u32stack_pop(s);
}

void u32stack_push(u32stack** s, u32 e)
{
	u32stack* t = (u32stack*) malloc(sizeof(u32stack));
	assert(t);
	t->head = e;
	t->tail = *s;
	*s = t;
}

u32 u32stack_pop(u32stack** s)
{
	u32 tos     = (*s)->head;
	u32stack* t = (*s)->tail;
	free(*s);
	*s = t;
	return tos;
}
