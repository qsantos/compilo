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

#include "set.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

Set* Set_New(u32 n)
{
	Set* s = (Set*) malloc(sizeof(Set));
	assert(s);
	s->n = n;
	s->nw = n / 32;
	if (s->nw * 32 < n)
		s->nw++;
	s->obj = (u32*) calloc(s->nw, sizeof(u32));
	assert(s->obj);
	return s;
}

void Set_Delete(Set* s)
{
	assert(s);
	free(s->obj);
	free(s);
}

Set* Set_Copy(Set* s)
{
	assert(s);
	Set* c = Set_New(s->n);
	memcpy(c->obj, s->obj, sizeof(u32) * s->nw);
	return c;
}

bool Set_IsIn(Set* s, u32 a)
{
	assert(s);
	return (s->obj[a / 32] & (1 << (a%32))) != 0;
}

bool Set_Cmp(Set* a, Set* b)
{
	assert(a);
	assert(b);
	
	if (a->n != b->n)
		return false;
	for (u32 i = 0; i < a->nw; i++)
		if (a->obj[i] != b->obj[i])
			return true;
	return false;
}

void Set_Add(Set* s, u32 a)
{
	assert(s);
	s->obj[a / 32] |= 1 << (a % 32);
}

void Set_Remove(Set* s, u32 a)
{
	assert(s);
	s->obj[a / 32] &= ~(1 << (a % 32));
}

Set* Set_Union(Set* a, Set* b)
{
	assert(a);
	assert(b);
	assert(a->n == b->n);
	
	Set* s = Set_Copy(a);
	for (u32 i = 0; i < a->nw; i++)
		s->obj[i] |= b->obj[i];
	return s;
}

Set* Set_UnionDiff(u32stack* a, Set* b, u32stack* c)
{
	assert(b);
	
	Set* s = Set_Copy(b);
	while (c)
	{
		Set_Remove(s, c->head);
		c = c->tail;
	}
	while (a)
	{
		Set_Add(s, a->head);
		a = a->tail;
	}
	
	return s;
}

Set* Set_UnionNull(Set* a, Set* b, u32 n)
{
	if (a)
		return b ? Set_Union(a, b) : Set_Copy(a);
	else
		return b ? Set_Copy(b)     : Set_New(n);
}

void Set_Print(Set* s)
{
	for (u32 i = 0; i < s->n; i++)
		if (Set_IsIn(s, i))
			printf("%lu, ", i);
}
