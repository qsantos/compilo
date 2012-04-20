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

#include "salmon.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* XXX: DEBUG */
/*void Set_Display(Set* s)
{
	for (u32 i = 0; i < s->n; i++)
		if (s->obj[i])
			printf("%lu, ", i);
	printf("\n");
}

void Salmon_VivacityDebug(u32 n, ASM* a)
{
	for (u32 i = 0; i < n; i ++)
	{
		printf("-- %lu\n", i);
		printf("in: ");
		Set_Display(a->code[i].s.in);
		printf("out: ");
		Set_Display(a->code[i].s.out);
		printf("def: ");
		Set_Display(a->code[i].s.def);
		printf("use: ");
		Set_Display(a->code[i].s.use);
	}
}*/

void Salmon_BuildFlow(ASM* a)
{
	u32 n = a->n_code;
	u32stack* s;
	for (u32 i = 0; i < n; i ++)
	{
		a->code[i].s.jmp  = -1;
		a->code[i].s.use  = Set_New(n);
		a->code[i].s.out  = Set_New(n);
		a->code[i].s.def  = Set_New(n);
		
		switch (a->code[i].insn)
		{
		case INSN_SET:
			Set_Append(a->code[i].v.r.r0, a->code[i].s.def);
			break;
		case INSN_MOV:  case INSN_NOT: case INSN_LNOT:
			Set_Append(a->code[i].v.r.r0, a->code[i].s.def);
			Set_Append(a->code[i].v.r.r1, a->code[i].s.use);
			break;
		case INSN_AND:  case INSN_OR:  case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:   case INSN_NEQ: case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD:  case INSN_SUB: case INSN_MUL: case INSN_DIV:  case INSN_MOD:
			Set_Append(a->code[i].v.r.r0, a->code[i].s.def);
			Set_Append(a->code[i].v.r.r1, a->code[i].s.use);
			Set_Append(a->code[i].v.r.r2, a->code[i].s.use);
			break;
		case INSN_JMP:  case INSN_JZ:  case INSN_JNZ:
			a->code[i].s.jmp = a->labels[a->code[i].v.r.r0];
			break;
		case INSN_CALL:
			s = a->code[i].v.p->tail;
			while (s)
			{
				Set_Append(s->head, a->code[i].s.use);
				s = s->tail;
			}
			break;
		case INSN_RET:
			Set_Append(0, a->code[i].s.def);
			break;
		default:
			break;
		}
		
		a->code[i].s.in = Set_Copy(a->code[i].s.use);
	}
}

void Salmon_Vivacity(ASM* a)
{
	u32 n = a->n_code;	
	bool changed = true;
	while (changed)
	{
		changed = false;
		u32 i = n - 1;
		do
		{
			Set* tmp;
			tmp = Set_Diff(a->code[i].s.out, a->code[i].s.def);
			Set* in = Set_Union(a->code[i].s.use, tmp);
			Set_Delete(tmp);
			
			changed = changed || Set_Cmp(in, a->code[i].s.in);
			Set_Delete(a->code[i].s.in);
			a->code[i].s.in = in;
			
			Set* out;
			if (i < (n-1))
			{
				if (a->code[i].s.jmp != -1)
					out = Set_Union(a->code[i+1].s.in, a->code[a->code[i].s.jmp].s.in);
				else
					out = Set_Copy(a->code[i+1].s.in);
			}
			else
			{
				if (a->code[i].s.jmp != -1)
					out = Set_Copy(a->code[a->code[i].s.jmp].s.in);
				else
					out = Set_New(n);
			}
			
			changed = changed || Set_Cmp(out, a->code[i].s.out);
			Set_Delete(a->code[i].s.out);
			a->code[i].s.out = out;
			
			i--;
		} while (i);
	}
	
	//Salmon_VivacityDebug(n, a);
}

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
	
	for (u32 i = 0; i < a->n_code; i++)
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
		case INSN_AND: case INSN_OR:  case INSN_XOR: case INSN_LAND: case INSN_LOR:
		case INSN_EQ:  case INSN_NEQ: case INSN_LE:  case INSN_LT:   case INSN_GE:  case INSN_GT:
		case INSN_ADD: case INSN_SUB: case INSN_MUL: case INSN_DIV:  case INSN_MOD:
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

RegAlloc* Salmon_RegAlloc(IntGraph* g, u32 k)
{
	/* Allocation des registres */
	RegAlloc* ra = (RegAlloc*) calloc(g->n, sizeof(RegAlloc));
	assert(ra);
	bool* colored = (bool*) calloc(g->n, sizeof(bool));
	assert(ra);
	
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
		
		/* 1er cas: on peut retirer un sommet */
		bool found = false;
		u32 v = 0;
		while (!found && v < g->n)
		{
			if (!g->dead[v] && g->d[v] < k && !g->move[v])
			{
				IntGraph_Simplify(s[t], v);
				rem--;
				found = true;
				op[t].kind = SIMPLIFY;
				op[t].v1   = v;
			}
			else
				v++;
		}
		if (found)
			continue;

		/* 2ème cas: on peut fusionner deux sommets */
		found = false;
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
						IntGraph_Coalesce(s[t], i, j);
						rem--;
						found = true;
						op[t].kind = COALESCE;
						op[t].v1   = i;
						op[t].v2   = j;
					}
				}
			}
		}
		if (found)
			continue;
		
		/* 3ème cas: on peut supprimer une arête de préférence */
		found = false;
		v = 0;
		while (!found && v < g->n)
		{
			if (!g->dead[v] && g->move[v])
			{
				IntGraph_DeletePref(s[t], v);
				found      = true;
				op[t].kind = DELPREFS;
				op[t].v1   = v;
			}
			else
				v++;
		}
		if (found)
			continue;
		
		/* 4ème cas: spilling */
		found = false;
		v = 0;
		while (!found && v < g->n)
		{
			if (!g->dead)
			{
				IntGraph_Simplify(s[t], v);
				ra[v].spilled = true;
				colored[v]    = true;
				found         = true;
				op[t].kind    = SPILLING;
			}
		}
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
		switch (op[i].kind)
		{
		case SIMPLIFY:
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
			break;
			
		case COALESCE:
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
			break;
			
		case DELPREFS:
			/* FIXME: Nothing to do ? */
			break;
			
		default:
			break;
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
