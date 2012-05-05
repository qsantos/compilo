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

#include "intgraph.h"

#include <stdlib.h>
#include <assert.h>

#define EDGE(U, V) (g->e[(V) * g->n + (U)])
IntGraph* IntGraph_New(u32 n)
{
	IntGraph* g = (IntGraph*) malloc(sizeof(IntGraph)); assert(g);
	
	g->n    = n;
	g->d    = (u32*)  calloc(n,     sizeof(u32));  assert(g->d);
	g->move = (bool*) calloc(n,     sizeof(bool)); assert(g->move);
	g->e    = (edge*) calloc(n * n, sizeof(edge)); assert(g->e);
	g->dead = (bool*) calloc(n,     sizeof(bool)); assert(g->dead);
	
	return g;
}

void IntGraph_Delete(IntGraph* g)
{
	free(g->dead);
	free(g->e);
	free(g->move);
	free(g->d);
	free(g);
}

bool IntGraph_AddInterf(IntGraph* g, u32 i, u32 j)
{
	if (i != j && !EDGE(i, j).interf)
	{
		EDGE(i, j).interf = true;
		EDGE(j, i).interf = true;
		g->d[i]++;
		g->d[j]++;
		return true;
	}
	else
		return false;
}

bool IntGraph_DelInterf(IntGraph* g, u32 i, u32 j)
{
	if (EDGE(i, j).interf)
	{
		EDGE(i, j).interf = false;
		EDGE(j, i).interf = false;
		g->d[i]--;
		g->d[j]--;
		return true;
	}
	else
		return false;
}

bool IntGraph_AddMove(IntGraph* g, u32 i, u32 j)
{
	if (i != j && !EDGE(i, j).pref)
	{
		EDGE(i, j).pref = true;
		EDGE(j, i).pref = true;
		g->move[i] = true;
		g->move[j] = true;
		return true;
	}
	else
		return false;
}

bool IntGraph_DelMove(IntGraph* g, u32 i, u32 j)
{
	if (EDGE(i, j).pref)
	{
		EDGE(i, j).pref = false;
		EDGE(j, i).pref = false;
		g->move[i] = false;
		g->move[j] = false;
		for (u32 k = 0; k < g->n; k++)
			if (!g->dead[k])
			{
				if (EDGE(i, k).pref)
					g->move[i] = true;
				if (EDGE(j, k).pref)
					g->move[j] = true;
			}
		return true;
	}
	else
		return false;
}

void IntGraph_Simplify(IntGraph* g, u32 v)
{
	g->dead[v] = true;
	for (u32 i = 0; i < g->n; i++)
		if (!g->dead[i] && EDGE(i, v).interf)
			g->d[i]--;
//	g->move[v] = false;
//	g->d[v]    = 0;
//	for (u32 i = 0; i < g->n; i++)
//		if (!g->dead[i])
//		{
//			IntGraph_DelInterf(g, i, v);
//			IntGraph_DelMove  (g, i, v);
//		} TODO
}

void IntGraph_Coalesce(IntGraph* g, u32 v1, u32 v2)
{
	for (u32 i = 0; i < g->n; i++)
	{
		if (IntGraph_DelInterf(g, v2, i))
			IntGraph_AddInterf(g, v1, i);
		if (IntGraph_DelMove(g, v2, i))
			IntGraph_AddMove(g, i, v1);
	}
	g->dead[v2] = true;
}

IntGraph* IntGraph_FromFlow(Flow* f)
{
	IntGraph* g = IntGraph_New(f->nr);
	u32stack* s;
	
	for (u32 i = 0; i < f->ns; i++)
	{
		s = f->s[i].def;
		while (s)
		{
			for (u32 j = 0; j < f->nr; j++)
				if (Set_IsIn(f->s[i].out, j) && (s32)j != f->s[i].mov)
					IntGraph_AddInterf(g, s->head, j);
			if (f->s[i].mov >= 0)
				IntGraph_AddMove(g, s->head, f->s[i].mov);
			s = s->tail;
		}
	}
	
	return g;
}
