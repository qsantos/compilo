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

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "types.h"

typedef struct
{
	string k;
	u32 u;
	u32 v;
} KValue;

typedef struct
{
	KValue* t;
	u32 size;
	u32 n_elements;
} HashTable;

HashTable* HashTable_new(u32 size);
void HashTable_delete(HashTable* ht);
u32 HashTable_find(HashTable* ht, cstring key, u32 unifier);

#endif
