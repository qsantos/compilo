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

void Set_Delete(Set* s)
{
	free(s->obj);
	free(s);
}

Set* Set_Union(Set* a, Set* b)
{
	assert(a->n == b->n);
	Set* s = Set_New(a->n);
	for (u32 i = 0; i < n; i ++)
		s = a->obj[i] || b->obj[i];
	return s;
}

Set* Set_Diff(Set* a, Set* b)
{
	assert(a->n == b->n);
	Set* s = Set_New(a->n);
	for (u32 i = 0; i < n; i ++)
		s = a->obj[i] && !b->obj[i];
	return s;
}
