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

#include "set.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

Set* Set_New(u32 n)
{
	Set* s = (Set*) malloc(sizeof(Set));
	assert(s);
	s->n = n;
	s->obj = (bool*) calloc(n, sizeof(bool));
	assert(s->obj);
	return s;
}

Set* Set_Singleton(u32 n, u32 a)
{
	Set* s = Set_New(n);
	s->obj[a] = true;
	return s;
}

Set* Set_Pair(u32 n, u32 a, u32 b)
{
	Set* s = Set_New(n);
	s->obj[a] = true;
	s->obj[b] = true;
	return s;
}

Set* Set_Copy(Set* s)
{
	Set* c = Set_New(s->n);
	memcpy(c->obj, s->obj, sizeof(bool) * s->n);
	return c;
}

void Set_Append(u32 a, Set* s)
{
	s->obj[a] = true;
}

void Set_Delete(Set* s)
{
	free(s->obj);
	free(s);
}

Set* Set_Union(Set* a, Set* b)
{
	assert(a->n == b->n);
	Set* s = Set_New(a->n);
	for (u32 i = 0; i < a->n; i ++)
		s->obj[i] = a->obj[i] || b->obj[i];
	return s;
}

Set* Set_Diff(Set* a, Set* b)
{
	assert(a->n == b->n);
	Set* s = Set_New(a->n);
	for (u32 i = 0; i < a->n; i ++)
		s->obj[i] = a->obj[i] && !b->obj[i];
	return s;
}

bool Set_Cmp(Set* a, Set* b)
{
	if (a->n != b->n)
		return false;
	bool diff = false;
	u32 i = 0;
	while (!diff && i < a->n)
	{
		diff = (a->obj[i] != b->obj[i]);
		i++;
	}
	return diff;
}
