/*
 *   A Tiny C to SPIM Compiler
 *   Copyright (C) 2012 Thomas GREGOIRE, Quentin SANTOS
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
 */

#include "static.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

context* Context_New(u32 size)
{
	context* c = (context*)malloc(sizeof(context));
	
	c->ht = HashTable_New(size);
	
	c->l2g = (u32stack**)malloc(sizeof(u32stack*) * size);
	memset(c->l2g, 0, sizeof(u32stack*) * size);
	
	c->st = (symbol*) malloc(sizeof(symbol) * size);
	memset(c->st, 0, sizeof(symbol) * size);
	
	c->err = false;
	c->defined = NULL;
	c->forget  = NULL;
	return c;
}

void Context_Delete(context* c)
{
	u32stack_delete(&c->forget);
	u32stack_delete(&c->defined);
	
	for (u32 i = 0; i < c->ht->size; i++)
		u32stack_delete(&c->l2g[i]);
	free(c->l2g);
	
	HashTable_Delete(c->ht);
	free(c->st);
	free(c);
}

void Context_BeginScope(context* c)
{
	u32stack_push(&c->forget, 0);
	c->depth++;
}

void Context_EndScope(context* c)
{
	u32 k = u32stack_pop(&c->forget);
	while (k)
	{
		u32 localId  = u32stack_pop(&c->defined);
		u32 globalId = u32stack_pop(&c->l2g[localId]);
		c->st[globalId].isDefined  = false;
		k--;
	}
	c->depth--;
}

symbol* Context_Declare(context* c, cstring name)
{
	u32 localId = HashTable_Find(c->ht, name);
	static u32 globalId = 0; // TODO
	
	u32stack_push(&c->l2g[localId], globalId);
	u32stack_push(&c->defined,      localId);
	c->forget->head++;
	
	symbol* symb = &c->st[globalId];
	symb->id     = globalId;
	symb->depth  = c->depth;
	
	globalId++;
	
	return symb;
}

symbol* Context_Get(context* c, cstring name)
{
	if (!HashTable_Exists(c->ht, name))
		return NULL;
	
	u32 localId  = HashTable_Find(c->ht, name);
	if (!c->l2g[localId])
		return NULL;
	
	u32 globalId = c->l2g[localId]->head;
	return &c->st[globalId];
}

bool Context_CanDeclare(context* c, cstring name)
{
	symbol* symb = Context_Get(c, name);
	return !symb || symb->depth < c->depth;
}

void Static_Error(context* c, position* pos, cstring format, ...)
{
	va_list va;
	va_start(va, format);
	fprintf(stderr, "Line %d, character %d: ", pos->first_line, pos->first_column);
	vfprintf(stderr, format, va);
	fprintf(stderr, "\n");
	va_end(va);
	c->err = true;
}
