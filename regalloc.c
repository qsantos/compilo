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

#define EDGE(G, U, V) ((G)->e[(V) * g->n + (U)])
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

IntGraph* IntGraph_Copy(IntGraph* g)
{
	IntGraph* c = IntGraph_New(g->n);
	memcpy(c->d,    g->d,    sizeof(u32)  * g->n);
	memcpy(c->move, g->move, sizeof(bool) * g->n);
	memcpy(c->e,    g->e,    sizeof(edge) * g->n * g->n);
	memcpy(c->dead, g->dead, sizeof(bool) * g->n);
	return c;
}

bool IntGraph_AddInterf(IntGraph* g, u32 i, u32 j)
{
	if (i != j && !EDGE(g, i, j).interf)
	{
		EDGE(g, i, j).interf = true;
		EDGE(g, j, i).interf = true;
		g->d[i]++;
		g->d[j]++;
		return true;
	}
	else
		return false;
}

bool IntGraph_DelInterf(IntGraph* g, u32 i, u32 j)
{
	if (EDGE(g, i, j).interf)
	{
		EDGE(g, i, j).interf = false;
		EDGE(g, j, i).interf = false;
		g->d[i]--;
		return true;
	}
	else
		return false;
}

bool IntGraph_AddMove(IntGraph* g, u32 i, u32 j)
{
	if (i != j && !EDGE(g, i, j).pref)
	{
		EDGE(g, i, j).pref = true;
		EDGE(g, j, i).pref = true;
		g->move[i] = true;
		g->move[j] = true;
		return true;
	}
	else
		return false;
}

bool IntGraph_DelMove(IntGraph* g, u32 i, u32 j)
{
	if (EDGE(g, i, j).pref)
	{
		EDGE(g, i, j).pref = false;
		EDGE(g, j, i).pref = false;
		g->move[i] = false;
		g->move[j] = false;
		for (u32 k = 0; k < g->n; k++)
			if (!g->dead[k])
			{
				if (k != j && EDGE(g, i, k).pref)
				{
					g->move[i] = true;
				}
				if (k != i && EDGE(g, j, k).pref)
				{
					g->move[j] = true;
				}
			}
		return true;
	}
	else
	{
		return false;
	}
}

void IntGraph_Simplify(IntGraph* g, u32 v)
{
	g->dead[v] = true;
	g->move[v] = false;
	g->d[v]    = 0;
	for (u32 i = 0; i < g->n; i++)
		if (i != v && !g->dead[i])
		{
			IntGraph_DelInterf(g, i, v);
			IntGraph_DelMove  (g, i, v);
		}
}

void IntGraph_Coalesce(IntGraph* g, u32 v1, u32 v2)
{
	for (u32 i = 0; i < g->n; i++)
		if (!g->dead[i])
		{
			if (IntGraph_DelInterf(g, v2, i))
				IntGraph_AddInterf(g, v1, i);
			
			if (IntGraph_DelMove(g, v2, i))
				IntGraph_AddMove(g, i, v1);
		}
	g->dead[v2] = true;
}

IntGraph* Salmon_Interference(ASM* a)
{
	IntGraph* g = IntGraph_New(a->n_regs);
	
	for (u32 i = 0; i < a->n_code; i++)
	{
		switch (a->code[i].insn)
		{
		case INSN_MOV:
			for (u32 j = 0; j < a->n_regs; j++)
				if (a->code[i].s.out->obj[j])
					IntGraph_AddMove(g, a->code[i].v.r.r0, j);
			break;
		case INSN_SET:
		case INSN_NOT: case INSN_LNOT:
		case INSN_AND: case INSN_OR:   case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:  case INSN_NEQ:  case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD: case INSN_SUB:  case INSN_MUL: case INSN_DIV:  case INSN_MOD:
			for (u32 j = 0; j < a->n_regs; j++)
				if (a->code[i].s.out->obj[j])
					IntGraph_AddInterf(g, a->code[i].v.r.r0, j);
			break;
		default:
			break;
		}
	}
	
	return g;
}

typedef struct
{
	enum
	{
		SIMPLIFY,
		COALESCE,
		DELPREFS,
		SPILLING,
	} kind;
	u32 v1;
	u32 v2;
} IRC_Op;

/* Confer https://en.wikipedia.org/wiki/Register_allocation#Iterated_Register_Coalescing */ 

/* 1. Tries to remove a vertex with degree lower than k */
bool RA_DelVertex(IntGraph* g, IRC_Op* op, u32 k)
{
	for (u32 v = 0; v < g->n; v++)
		if (!g->dead[v] && g->d[v] < k && !g->move[v])
		{
			IntGraph_Simplify(g, v);
			op->kind = SIMPLIFY;
			op->v1   = v;
//			printf("+ Simplify(%lu)\n", v);
			return true;
		}
//		else
//			printf("%lu -> %u, %lu, %u\n", v, g->dead[v], g->d[v], g->move[v]); // TODO
	return false;
}

/* 2. Tries to contract an edge with combined degree lower than k */
bool RA_DelEdge(IntGraph* g, IRC_Op* op, u32 k)
{
	for (u32 i = 0; i < g->n; i++)
		if (!g->dead[i])
			for (u32 j = i+1; j < g->n; j++)
				if (!g->dead[j] && EDGE(g, i, j).pref)
				{
					u32 count = 0;
					for (u32 k = 0; k < g->n; k++)
					{
						bool n = !g->dead[k];
						n = n && k != j && EDGE(g, i, k).interf;
						n = n && k != i && EDGE(g, j, k).interf;
						if (n)
							count++;
					}
					
					if (count < k)
					{
						IntGraph_Coalesce(g, i, j);
						op->kind = COALESCE;
						op->v1   = i;
						op->v2   = j;
//						printf("+ Coalesce(%lu, %lu)\n", i, j);
						return true;
					}
				}
	return false;
}

/* 3. Tries to remove a preference edge */
bool RA_DelPref(IntGraph* g, IRC_Op* op)
{
	for (u32 v = 0; v < g->n; v++)
		if (!g->dead[v] && g->move[v])
		{
			for (u32 i = 0; i < g->n; i++)
				if (!g->dead[i])
					IntGraph_DelMove(g, v, i);
			assert(!g->move[v]);
			op->kind = DELPREFS;
			op->v1   = v;
//			printf("+ Delprefs(%lu)\n", v);
			return true;
		}
	return false;
}

/* 4. Spilling */
bool RA_Spill(IntGraph* g, IRC_Op* op, RegAlloc* ra)
{
	for (u32 v = 0; v < g->n; v++)
		if (!g->dead[v])
		{
			IntGraph_Simplify(g, v);
			ra[v].spilled = true;
			op->kind      = SPILLING;
			return true;
		}
	return true; //           /!\.
}


RegAlloc* Salmon_RegAlloc(IntGraph* g, u32 k)
{
	RegAlloc* ra  = (RegAlloc*) calloc(g->n, sizeof(RegAlloc)); assert(ra);
	bool* colored = (bool*)     calloc(g->n, sizeof(bool));     assert(colored);
	
	// graph serie
	u32 t   = 0;
	u32 a_s = g->n;
	IntGraph** s = (IntGraph**) malloc(sizeof(IntGraph*) * a_s); assert(s);
	s[0] = IntGraph_Copy(g);
	
	// operation serie
	IRC_Op* op = (IRC_Op*) malloc(sizeof(IRC_Op) * a_s); assert(op);
	
	u32 rem = g->n; // remaining vertices
	while (rem)
	{
//		printf("================================\n");
//		printf("%lu remaining\n", rem);
//		for (u32 i = 0; i < s[t]->n; i++)
//			printf("s[t]->d[%.2lu] = %lu\n", i, s[t]->d[i]);
//		printf("\n");
	
		t++;
		// copy from last graph
		if (t == a_s)
		{
			a_s *= 2;
			s  = (IntGraph**) realloc(s,  sizeof(IntGraph*) * a_s); assert(s);
			op = (IRC_Op*)    realloc(op, sizeof(IRC_Op)    * a_s); assert(op);
		}
		s[t] = IntGraph_Copy(s[t-1]);
		
		     if (RA_DelVertex(s[t], &op[t], k))  rem--;
		else if (RA_DelEdge  (s[t], &op[t], k))  rem--;
		else if (RA_DelPref  (s[t], &op[t]))          ;
		else if (RA_Spill    (s[t], &op[t], ra)) rem--;
	}
	
	// local use
	bool* neighborColors = (bool*) calloc(k, sizeof(bool)); assert(neighborColors);
	
	u32 n_graphs = t;
	while (t)
	{
		memset(neighborColors, 0, sizeof(bool) * k);
		
//		printf("===== %lu ====\n", t);
//		for (u32 i = 0; i < g->n; i++)
//			for (u32 j = 0; j < g->n; j++)
//				if (EDGE(s[t-1], i, j).interf)
//					printf("I JUST FOUND ONE: %lu, %lu\n", i, j);
			
		if (op[t].kind == SIMPLIFY)
		{
			u32 v1 = op[t].v1;
//			printf("- Simplify(%lu)\n", v1);
			
			for (u32 i = 0; i < g->n; i++)
				if (!s[t-1]->dead[i] && i != v1 && EDGE(s[t-1], v1, i).interf && colored[i])
					neighborColors[ra[t].color] = true;
			
			for (u32 i = 0; i < k; i++)
				if (!neighborColors[i])
				{
					ra[v1].color = i;
//					printf("%lu got c%lu\n", v1, i);
					break;
				}
			
			colored[v1] = true;
		}
		else if (op[t].kind == COALESCE)
		{
			u32 v1 = op[t].v1;
			u32 v2 = op[t].v2;
//			printf("- Coalesce(%lu, %lu)\n", v1, v2);
			
/*
			for (u32 j = 0; j < g->n; j++)
				if (!s[i]->dead[j] && colored[j] && j != v1 && EDGE(s[i], v1, j).interf && j !=v2 && EDGE(s[i], v2, j).interf)
					neighborColors[ra[j].color] = true;
			
			for (u32 r = 0; r < k; r++)
				if (!neighborColors[r])
				{
					ra[v1].color = r;
					ra[v2].color = r;
					printf("%lu got c%lu\n", v1, r);
					printf("%lu got c%lu\n", v2, r);
					break;
				}
*/
			
//			colored[v1] = true;
//			colored[v2] = true;
			
			ra[v2].color = ra[v1].color;
//			printf("%lu got c%lu\n", v2, ra[v1].color);
			colored[v2] = true;
		}
		t--;
	}
	
	/* Cleaning */
	free(neighborColors);
	free(op);
	for (u32 i = 0; i <= n_graphs; i++)
		IntGraph_Delete(s[i]);
	free(s);
	free(colored);
	
//	printf("=== Register Allocation ===\n");
//	for (u32 i = 0; i < g->n; i++)
//		printf("%lu -> %lu\n", i, ra[i].color);
//	printf("=========\n");
	return ra;
}
