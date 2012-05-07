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

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "types.h"

typedef struct
{
	string k;
	u32 v;
} KValue;

typedef struct
{
	KValue* t;
	u32 size;
	u32 n_elements;
} HashTable;

HashTable* HashTable_New   (u32);
void       HashTable_Delete(HashTable*);
bool       HashTable_Exists(HashTable*, cstring);
u32        HashTable_Find  (HashTable*, cstring);

#endif
