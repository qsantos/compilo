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

IntGraph* IntGraph_New(u32 n)
{
	IntGraph* g = (IntGraph*) malloc(sizeof(IntGraph));
	assert(g);
	
	g->n = n;
	g->d = (u32*) calloc(n, sizeof(u32));
	assert(g->d);
	g->move = (bool*) calloc(n, sizeof(bool));
	assert(g->move);
	g->e = (edge*) calloc(n * n, sizeof(edge));
	assert(g->e);
	g->dead = (bool*) calloc(n, sizeof(bool));
	assert(g->dead);
	
	return g;
}

void IntGraph_AddInterf(IntGraph* g, u32 i, u32 j)
{
	if (!g->e[i + j * g->n].interf)
	{
		g->e[i + j * g->n].interf = true;
		g->e[j + i * g->n].interf = true;
		g->d[i]++;
		g->d[j]++;
	}
}

void IntGraph_AddMove(IntGraph* g, u32 i, u32 j)
{
	if (!g->e[i + j * g->n].pref)
	{
		g->e[i + j * g->n].pref = true;
		g->e[j + i * g->n].pref = true;
		g->move[i] = true;
		g->move[j] = true;
	}
}

IntGraph* IntGraph_Copy(IntGraph* g)
{
	IntGraph* c = IntGraph_New(g->n);
	memcpy(c->d,    g->d,    sizeof(u32) * g->n);
	memcpy(c->move, g->move, sizeof(bool) * g->n);
	memcpy(c->e,    g->e,    sizeof(edge) * g->n * g->n);
	return c;
}

void IntGraph_Simplify(IntGraph* g, u32 v)
{
	g->dead[v] = true;
	g->move[v] = false;
	g->d[v]    = 0;
	for (u32 i = 0; i < g->n; i++)
	{
		if (!g->dead[i])
		{
			if (g->e[i + g->n * v].interf)
			{
				g->e[i + g->n * v].interf = false;
				g->e[v + g->n * i].interf = false;
				g->d[i]--;
			}
			
			if (g->e[i + g->n * v].pref)
			{
				g->e[i + g->n * v].pref = false;
				g->e[v + g->n * i].pref = false;
				g->move[i] = false;
				for (u32 j = 0; j < g->n; j++)
					g->move[i] |= g->e[i + g->n * j].pref;
			}
		}
	}
}

void IntGraph_Coalesce(IntGraph* g, u32 v1, u32 v2)
{
	g->dead[v2] = true;
	g->move[v2] = false;
	g->d[v2]    = 0;
	for (u32 i = 0; i < g->n; i++)
	{
		if (!g->dead[i])
		{
			if (g->e[i + g->n * v2].interf)
			{
				g->e[i  + g->n * v2].interf = false;
				g->e[v2 + g->n *  i].interf = false;
				if (!g->e[i + g->n * v1].interf)
				{
					g->e[i  + g->n * v1].interf = true;
					g->e[v1 + g->n *  i].interf = true;
					g->d[v1]++;
				}
				else
					g->d[i]--;
			}
			
			if (g->e[i + g->n * v2].pref)
			{
				g->e[i  + g->n * v2].pref = false;
				g->e[v2 + g->n *  i].pref = false;
				if (!g->e[i + g->n * v1].pref)
				{
					g->e[i  + g->n * v1].pref = true;
					g->e[v1 + g->n *  i].pref = true;
				}
			}
		}
	}
}

void IntGraph_DeletePref(IntGraph* g, u32 v)
{
	g->move[v] = false;
	for (u32 i = 0; i < g->n; i++)
	{
		if (!g->dead[i])
		{
			g->e[i + g->n * v].pref = false;
			g->e[v + g->n * i].pref = false;
			g->move[i] = false;
			for (u32 j = 0; j < g->n; j++)
				g->move[i] |= g->e[i + g->n * j].pref;
		}
	}
}

void IntGraph_Delete(IntGraph* g)
{
	free(g->dead);
	free(g->d);
	free(g->move);
	free(g->e);
	free(g);
}

IntGraph* Salmon_Interference(ASM* a)
{
	IntGraph* g = IntGraph_New(a->n_regs);
	
	for (u32 i = 0; i < a->n_regs-1; i++)
	{
		switch (a->code[i].insn)
		{
		case INSN_MOV:
			for (u32 j = 0; j < a->n_code; j++)
			{
				if (a->code[i].s.out->obj[j] || j != a->code[i].v.r.r1)
					IntGraph_AddMove(g, a->code[i].v.r.r0, j);
			}
			break;

		case INSN_SET:
		case INSN_NOT: case INSN_LNOT:
		case INSN_AND: case INSN_OR:   case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:  case INSN_NEQ:  case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD: case INSN_SUB:  case INSN_MUL: case INSN_DIV:  case INSN_MOD:
			for (u32 j = 0; j < a->n_code; j++)
			{
				if (a->code[i].s.out->obj[j])
					IntGraph_AddInterf(g, a->code[i].v.r.r0, j);
			}
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

/* 1er cas: on peut retirer un sommet */
bool RA_DelVertex(IntGraph* g, IRC_Op* op, u32 k)
{
	bool found = false;
	u32 v = 0;
	while (!found && v < g->n)
	{
		if (!g->dead[v] && g->d[v] < k && !g->move[v])
		{
			IntGraph_Simplify(g, v);
			found = true;
			op->kind = SIMPLIFY;
			op->v1   = v;
		}
		else
			v++;
	}
	return found;
}

/* 2ème cas: on peut fusionner deux sommets */
bool RA_DelEdge(IntGraph* g, IRC_Op* op, u32 k, bool* nb)
{
	bool found = false;
	for (u32 i = 0; i < g->n && !found; i++)
	{
		for (u32 j = i+1; j < g->n && !found; j++)
		{
			if (!g->dead[i] && !g->dead[j] && g->e[i + j * g->n].pref)
			{
				memset(nb, false, sizeof(bool) * g->n);
				
				for (u32 k = 0; k < g->n; k++)
					if (!g->dead[k] && g->e[i + k * g->n].interf && (k != j))
						nb[k] = true;
				for (u32 k = 0; k < g->n; k++)
					if (!g->dead[k] && g->e[j + k * g->n].interf && (k != i))
						nb[k] = true;
				
				u32 count = 0;
				for (u32 k = 0; k < g->n; k++)
					if (nb[k])
						count++;
				
				if (count < k)
				{
					IntGraph_Coalesce(g, i, j);
					found = true;
					op->kind = COALESCE;
					op->v1   = i;
					op->v2   = j;
				}
			}
		}
	}
	return found;
}

/* 3ème cas: on peut supprimer une arête de préférence */
bool RA_DelPref(IntGraph* g, IRC_Op* op)
{
	bool found = false;
	u32 v = 0;
	while (!found && v < g->n)
	{
		if (!g->dead[v] && g->move[v])
		{
			IntGraph_DeletePref(g, v);
			found      = true;
			op->kind = DELPREFS;
			op->v1   = v;
		}
		else
			v++;
	}
	return found;
}

/* 4ème cas: spilling */
void RA_Spill(IntGraph* g, IRC_Op* op, RegAlloc* ra, bool* colored)
{
	bool found = false;
	u32 v = 0;
	while (!found && v < g->n)
	{
		if (!g->dead)
		{
			IntGraph_Simplify(g, v);
			ra[v].spilled = true;
			colored[v]    = true;
			found         = true;
			op->kind      = SPILLING;
		}
	}
}


RegAlloc* Salmon_RegAlloc(IntGraph* g, u32 k)
{
	/* Allocation des registres */
	RegAlloc* ra = (RegAlloc*) calloc(g->n, sizeof(RegAlloc));
	assert(ra);
	bool* colored = (bool*) calloc(g->n, sizeof(bool));
	assert(colored);
	
	/* Suite des graphes obtenus en réduisant g de proche en proche */
	u32 t   = 0;    // numéro du graphe actuel
	u32 max = g->n; // nombre maxi de graphes actuellement alloués
	IntGraph** s = (IntGraph**) malloc(sizeof(IntGraph*) * max);
	assert(s);
	s[0] = IntGraph_Copy(g);
	
	/* Suite des opérations effectuées */
	IRC_Op* op = (IRC_Op*) malloc(sizeof(IRC_Op) * max);
	assert(op);
	
	/* Sert pour compter les "voisins" d'une arête (cas 2) */
	bool* nb = (bool*) malloc(sizeof(bool) * g->n);
	assert(nb);
	
	u32 rem = g->n; // nombre de sommets restant à traiter
	while (rem)
	{
		/* Copie du graphe actuel */
		t++;
		if (t == max)
		{
			max *= 2;
			s  = (IntGraph**) realloc(s,  sizeof(IntGraph*) * max);
			op = (IRC_Op*)    realloc(op, sizeof(IRC_Op*)   * max);
			assert(s);
		}
		s[t] = IntGraph_Copy(s[t-1]);

		/* Les 4 cas...  */
		if (RA_DelVertex(s[t], &op[t], k))     { rem --; continue; }
		if (RA_DelEdge(s[t],   &op[t], k, nb)) { rem --; continue; }
		if (RA_DelPref(s[t],   &op[t]))        {         continue; }
		RA_Spill(s[t], &op[t], ra, colored);
		rem--;
	}
	
	/* Tableau utilisé localement pour déterminer quelle couleur
	 * peut être affectée à un sommet ou à une arête */
	bool* col = (bool*) calloc(k, sizeof(bool));
	assert(col);
	
	bool found;
	u32 r;
	for (u32 i = t; i > 0; i--)
	{
		memset(col, 0, sizeof(bool) * k);
		
		u32 v1;
		u32 v2;
		if (op[i].kind == SIMPLIFY)
		{
			v1 = op[i].v1;
			for (u32 j = 0; j < g->n; j++)
			{
				if (!s[i]->dead[j] && colored[j] && s[i]->e[v1 + g->n * j].interf)
					col[ra[j].color] = true;
			}
			
			found = false;
			r = 0;
			while (!found && (r < k))
			{
				if (!col[r])
					found = true;
				else
					r++;
			}
			
			ra[v1].color = r;
		}
		else if (op[i].kind == COALESCE)
		{
			v1 = op[i].v1;
			v2 = op[i].v2;
			for (u32 j = 0; j < g->n; j++)
			{
				if (!s[i]->dead[j] && colored[j])
				{
					if ((j != v2) && s[i]->e[v1 + g->n * j].interf)
						col[ra[j].color] = true;
					if ((j != v1) && s[i]->e[v2 + g->n * j].interf)
						col[ra[j].color] = true;
				}
			}
			
			found = false;
			r = 0;
			while (!found && (r < k))
			{
				if (!col[r])
					found = true;
				else
					r++;
			}
			
			ra[v1].color = r;
			ra[v2].color = r;
		}
	}
	
	/* Ménage... */
	free(col);
	free(nb);
	free(op);
	for (u32 i = 0; i <= t; i++)
		IntGraph_Delete(s[i]);
	free(s);
	free(colored);
	
	/* A fini */
	return ra;
}
