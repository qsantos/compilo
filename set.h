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

#ifndef SET_H
#define SET_H

#include "types.h"
#include "u32stack.h"

typedef struct
{
	u32  n;
	u32  nw;
	u32* obj;
} Set;

Set* Set_New      (u32);
void Set_Delete   (Set*);
Set* Set_Copy     (Set*);
bool Set_IsIn     (Set*, u32);
bool Set_Cmp      (Set*, Set*);

void Set_Add      (Set*, u32);
void Set_Remove   (Set*, u32);
Set* Set_Union    (Set*, Set*);
Set* Set_UnionDiff(u32stack*, Set*, u32stack*);
Set* Set_UnionNull(Set*, Set*, u32);

void Set_Print    (Set*);

#endif
