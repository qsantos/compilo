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

#include "regalloc.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Confer https://en.wikipedia.org/wiki/Register_allocation#Iterated_Register_Coalescing */ 

#define MASK_COALESCE (0xF000000000000000)
#define EDGE(U, V) (g->e[(V) * g->n + (U)])
RegAlloc* IntGraph_RegAlloc(IntGraph* g, u32 k)
{
	RegAlloc* ra  = (RegAlloc*) calloc(g->n, sizeof(RegAlloc)); assert(ra);
	u32stack* vertices = NULL;
	
	/* Stack vertices which will be colored */
	u32 rem = g->n;
	while (rem)
	{
		bool done = false;
		/* 1. Tries to remove a vertex with degree lower than k */
		for (u32 v = 0; !done && v < g->n; v++)
			if (!g->dead[v] && g->d[v] < k && !g->move[v])
			{
				IntGraph_Simplify(g, v);
				u32stack_push(&vertices, v);
				rem--;
				done = true;
			}
		
		/* 2. Tries to contract an edge with combined degree lower than k */
/*
		for (u32 i = 0; !done && i < g->n; i++)
			if (!g->dead[i])
				for (u32 j = i+1; !done && j < g->n; j++)
					if (!g->dead[j] && EDGE(i, j).pref)
					{
						u32 count = 0;
						for (u32 k = 0; k < g->n; k++)
						{
							bool n = !g->dead[k];
							n = n && k != j && EDGE(i, k).interf;
							n = n && k != i && EDGE(j, k).interf;
							if (n)
								count++;
						}
						
						if (count < k)
						{
							IntGraph_Coalesce(g, i, j);
							u32stack_push(&vertices, i);
							u32stack_push(&vertices, j | MASK_COALESCE); 
							rem--;
							done = true;
						}
					}
*/
		
		/* 3. Tries to remove a preference edge */
		for (u32 v = 0; !done && v < g->n; v++)
			if (!g->dead[v] && g->move[v])
			{
				for (u32 i = 0; i < g->n; i++)
					if (!g->dead[i])
						IntGraph_DelMove(g, v, i);
				done = true;
			}
		
		/* 4. Spilling */
		for (u32 v = 0; !done && v < g->n; v++)
			if (!g->dead[v])
			{
				IntGraph_Simplify(g, v);
				ra[v].spilled = true;
				rem--;
				done = true;
			}
	}
	
	/* Assign colors */
	bool* neighborColors = (bool*) calloc(k,    sizeof(bool)); assert(neighborColors);
	bool* colored        = (bool*) calloc(g->n, sizeof(bool)); assert(colored);
	while (vertices)
	{
		u32 v = u32stack_pop(&vertices);
		if (v & MASK_COALESCE)
		{
			ra[v ^ MASK_COALESCE].color = ra[u32stack_pop(&vertices)].color;
		}
		else
		{
			for (u32 i = 0; i < g->n; i++)
				if (!g->dead[i] && i != v && EDGE(v, i).interf && colored[i])
					neighborColors[ra[i].color] = true;
			
			for (u32 i = 0; i < k; i++)
				if (!neighborColors[i])
				{
					ra[v].color = i;
					break;
				}
			memset(neighborColors, 0, sizeof(bool) * k);
		}
		g->dead[v] = false;
		colored[v] = true;
	}
	free(neighborColors);
	free(colored);
	
	return ra;
}

RegAlloc* ASM_RegAlloc(ASM* a, u32 s, u32 e, Context* c, u32 k)
{
	Flow* f = Flow_Build(a, s, e, c);
	Flow_Vivacity(f);
	
	IntGraph* g = IntGraph_FromFlow(f);
	RegAlloc* r = IntGraph_RegAlloc(g, k);
	
	IntGraph_Delete(g);
	Flow_Delete(f);
	
	return r;
}
